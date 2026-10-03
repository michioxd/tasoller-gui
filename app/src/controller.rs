use crate::protocol::{self, Config, Mapping, Profiles};
use crate::{AppWindow, Settings, keymap_file, transport};
use slint::ComponentHandle;
use std::sync::mpsc::{self, Receiver, SyncSender};

enum Action {
    Discover,
    Connect(String),
    Disconnect,
    Read,
    Apply(Config),
    ApplyAll(Config, Profiles),
    ApplyLighting(Config),
    ApplyMap(Profiles),
    Export(Profiles),
    Import,
    Save(Config, Profiles),
    Reset,
}

struct Update {
    ports: Option<Vec<String>>,
    info: Option<String>,
    config: Option<Config>,
    map: Option<Profiles>,
    import_map: Option<keymap_file::Imported>,
    connected: bool,
    message: String,
    error: bool,
    operation: bool,
    lighting_only: bool,
}

struct Session {
    port: Box<dyn serialport::SerialPort>,
    config: Config,
    map: Profiles,
}

impl Session {
    fn send(&mut self, command: u8, config: Option<Config>) -> Result<Vec<u8>, String> {
        transport::exchange(
            &mut *self.port,
            command,
            &protocol::request(command, config)?,
        )
    }

    fn read(&mut self) -> Result<Config, String> {
        let reply = self.send(protocol::GET_CONFIG, None)?;
        self.config = Config::decode(&reply[2..])?;
        Ok(self.config)
    }

    fn read_map(&mut self) -> Result<Profiles, String> {
        let reply = self.send(protocol::GET_KEYMAP, None)?;
        let keys = protocol::keymap(&reply[2..])?;
        let reply = self.send(protocol::GET_MODES, None)?;
        let mut banks = [[0; 32]; 6];
        for (mode, bank) in banks.iter_mut().enumerate() {
            let reply =
                transport::exchange(&mut *self.port, protocol::GET_PROFILE, &[1, mode as u8])?;
            bank.copy_from_slice(&reply[2..]);
        }
        self.map = Profiles {
            banks,
            shared: keys[32..].try_into().map_err(|_| "invalid shared keys")?,
            mode: reply[2],
            divider: reply[3],
        }
        .validate()?;
        Ok(self.map)
    }
}

fn perform(action: Action, session: &mut Option<Session>) -> Result<Update, String> {
    if let Action::ApplyAll(config, profiles) = action {
        config.encode()?;
        profiles.validate()?;
        let maps = perform(Action::ApplyMap(profiles), session)?;
        let mut update = perform(Action::Apply(config), session)?;
        update.map = maps.map;
        return Ok(update);
    }
    let mut update = Update {
        ports: None,
        info: None,
        config: None,
        map: None,
        import_map: None,
        connected: session.is_some(),
        message: String::new(),
        error: false,
        operation: true,
        lighting_only: false,
    };
    match action {
        Action::Discover => {
            let ports = transport::discover()?;
            update.message = format!("{} compatible device(s) found - USB 0CA3:0021", ports.len());
            update.ports = Some(ports);
        }
        Action::Connect(name) => {
            *session = None;
            let mut port = transport::open(&name)?;
            transport::exchange(&mut *port, protocol::REPORT_DISABLE, &[])?;
            let reply = transport::exchange(&mut *port, protocol::GET_INFO, &[1])?;
            let info = protocol::info(&reply[2..])?;
            let reply = transport::exchange(&mut *port, protocol::GET_CONFIG, &[1])?;
            let config = Config::decode(&reply[2..])?;
            let reply = transport::exchange(&mut *port, protocol::GET_KEYMAP, &[1])?;
            let keys = protocol::keymap(&reply[2..])?;
            let reply = transport::exchange(&mut *port, protocol::GET_MODES, &[1])?;
            let map = Mapping {
                keys,
                mode: reply[2],
                divider: reply[3],
            }
            .validate()?;
            let mut active = Session {
                port,
                config,
                map: Profiles {
                    banks: [[0; 32]; 6],
                    shared: [0; 8],
                    mode: map.mode,
                    divider: map.divider,
                },
            };
            let map = active.read_map()?;
            *session = Some(active);
            update.config = Some(config);
            update.map = Some(map);
            update.connected = true;
            update.info = Some(info.clone());
            update.message = format!("Connected to {name} - {info}");
        }
        Action::Disconnect => {
            *session = None;
            update.connected = false;
            update.message = "Disconnected.".into();
        }
        Action::Export(_) | Action::Import => {
            let result = match action {
                Action::Export(map) => keymap_file::export(map).map(|saved| {
                    update.message = if saved {
                        "Keymap exported."
                    } else {
                        "Export cancelled."
                    }
                    .into();
                }),
                Action::Import => keymap_file::import().map(|map| {
                    update.import_map = map;
                    update.message = if map.is_some() {
                        "Keymap imported into draft; apply to RAM when ready."
                    } else {
                        "Import cancelled."
                    }
                    .into();
                }),
                _ => unreachable!(),
            };
            if let Err(error) = result {
                update.error = true;
                update.message = error;
            }
        }
        action => {
            let session = session
                .as_mut()
                .ok_or("not connected; connect and read first")?;
            match action {
                Action::Read => {
                    update.config = Some(session.read()?);
                    update.map = Some(session.read_map()?);
                    update.message = "RAM settings read.".into();
                }
                Action::Apply(config) | Action::ApplyLighting(config) => {
                    update.lighting_only = matches!(action, Action::ApplyLighting(_));
                    let config = if update.lighting_only {
                        crate::controller::config(lighting_settings(
                            settings(config),
                            settings(session.read()?),
                        ))?
                    } else {
                        config
                    };
                    session.send(protocol::SET_CONFIG, Some(config))?;
                    if session.read()? != config {
                        return Err(
                            "RAM readback differs from requested settings; outcome unknown".into(),
                        );
                    }
                    update.config = Some(config);
                    update.message = "Saved to RAM!".into();
                }
                Action::ApplyMap(map) => {
                    map.validate()?;
                    for (mode, bank) in map.banks.iter().enumerate() {
                        let mut payload = vec![1, mode as u8];
                        payload.extend(bank);
                        transport::exchange(&mut *session.port, protocol::SET_PROFILE, &payload)?;
                    }
                    transport::exchange(
                        &mut *session.port,
                        protocol::SET_KEYMAP,
                        &map.mapping().request()?,
                    )?;
                    if session.read_map()? != map {
                        return Err("RAM keymap readback differs; outcome unknown".into());
                    }
                    update.map = Some(map);
                    update.message =
                        "All six profiles applied to RAM and verified; flash unchanged.".into();
                }
                Action::Save(config, map) => {
                    if config != session.config || map != session.map {
                        return Err("apply both edited settings and keymap before saving".into());
                    }
                    let current_config = session.read()?;
                    let current_map = session.read_map()?;
                    if current_config != config || current_map != map {
                        return Err("RAM settings or keymap changed; reread before saving".into());
                    }
                    session.send(protocol::SAVE_CONFIG, None)?;
                    update.message = "Flash save verified.".into();
                }
                Action::Reset => {
                    session.send(protocol::RESET_CONFIG, None)?;
                    update.config = Some(session.read()?);
                    update.map = Some(session.read_map()?);
                    update.message = "Defaults restored in RAM.".into();
                }
                _ => unreachable!(),
            }
        }
    }
    Ok(update)
}

fn worker(
    requests: Receiver<Action>,
    updates: SyncSender<Update>,
    previews: SyncSender<[u8; 33]>,
    ports: SyncSender<Result<Vec<String>, String>>,
) {
    let mut session: Option<Session> = None;
    let mut last_discovery = std::time::Instant::now();
    loop {
        let action = match requests.recv_timeout(std::time::Duration::from_millis(100)) {
            Ok(action) => action,
            Err(mpsc::RecvTimeoutError::Disconnected) => break,
            Err(mpsc::RecvTimeoutError::Timeout) => {
                // ponytail: poll idle device discovery once per second; use Windows
                // device notifications if immediate hotplug updates become necessary.
                if session.is_none()
                    && last_discovery.elapsed() >= std::time::Duration::from_secs(1)
                {
                    last_discovery = std::time::Instant::now();
                    let _ = ports.try_send(transport::discover());
                }
                if let Some(active) = session.as_mut() {
                    match active.send(protocol::GET_INPUT, None) {
                        Ok(reply) => {
                            if let Ok(input) = reply[2..].try_into() {
                                let _ = previews.try_send(input);
                            }
                        }
                        Err(error) => {
                            session = None;
                            if updates
                                .send(Update {
                                    ports: None,
                                    info: None,
                                    config: None,
                                    map: None,
                                    import_map: None,
                                    connected: false,
                                    error: true,
                                    operation: false,
                                    lighting_only: false,
                                    message: format!("Live input failed: {error}. Disconnected."),
                                })
                                .is_err()
                            {
                                break;
                            }
                        }
                    }
                }
                continue;
            }
        };
        let update = match perform(action, &mut session) {
            Ok(update) => update,
            Err(error) => {
                session = None;
                Update {
                    ports: None,
                    info: None,
                    config: None,
                    map: None,
                    import_map: None,
                    connected: false,
                    error: true,
                    operation: true,
                    lighting_only: false,
                    message: format!("{error}. Disconnected."),
                }
            }
        };
        if updates.send(update).is_err() {
            break;
        }
    }
}

fn selected_port(current: &str, ports: &[String]) -> String {
    if ports.iter().any(|port| port == current) {
        current.to_owned()
    } else {
        ports.first().cloned().unwrap_or_default()
    }
}

fn settings(config: Config) -> Settings {
    Settings {
        keyboard: config.keyboard,
        rainbow: config.rainbow,
        sensitivity: i32::from(config.sensitivity),
        left: i32::from(config.hues[0]),
        right: i32::from(config.hues[1]),
        ground: i32::from(config.hues[2]),
        active: i32::from(config.hues[3]),
        ground_brightness: i32::from(config.brightness[0]),
        tower_brightness: i32::from(config.brightness[1]),
    }
}

fn config(settings: Settings) -> Result<Config, String> {
    let byte = |value| u8::try_from(value).map_err(|_| "value must be 0-255".to_string());
    let hue = |value| u16::try_from(value).map_err(|_| "hue must be 0-359".to_string());
    let config = Config {
        keyboard: settings.keyboard,
        rainbow: settings.rainbow,
        sensitivity: byte(settings.sensitivity)?,
        hues: [
            hue(settings.left)?,
            hue(settings.right)?,
            hue(settings.ground)?,
            hue(settings.active)?,
        ],
        brightness: [
            byte(settings.ground_brightness)?,
            byte(settings.tower_brightness)?,
        ],
    };
    config.encode()?;
    Ok(config)
}

pub fn bind(ui: &AppWindow) -> Result<slint::Timer, std::io::Error> {
    let (send, requests) = mpsc::sync_channel(1);
    let (updates, receive) = mpsc::sync_channel(1);
    let (previews, preview_receive) = mpsc::sync_channel(1);
    let (ports, port_receive) = mpsc::sync_channel(1);
    std::thread::Builder::new()
        .name("tas-host-serial".into())
        .spawn(move || worker(requests, updates, previews, ports))?;
    ui.set_usage_choices(slint::ModelRc::new(slint::VecModel::from(
        std::iter::once(0)
            .chain(4..=0xa4)
            .map(|usage| slint::SharedString::from(protocol::usage_label(usage)))
            .collect::<Vec<_>>(),
    )));
    let weak = ui.as_weak();
    ui.on_select_input(move |slot| {
        if let Some(ui) = weak.upgrade() {
            if !(0..40).contains(&slot) {
                return;
            }
            let slot = if slot < 32 {
                protocol::representative(ui.get_keyboard_mode() as u8, slot as usize) as i32
            } else {
                slot
            };
            ui.set_selected_slot(slot);
            use slint::Model;
            let usage = ui.get_draft_map().row_data(slot as usize).unwrap_or(0);
            ui.set_usage_index(if usage == 0 { 0 } else { usage - 3 });
        }
    });
    let weak = ui.as_weak();
    ui.on_assign_usage(move |index| {
        if let Some(ui) = weak.upgrade() {
            if !ui.get_connected() || ui.get_busy() || !(0..=161).contains(&index) {
                return;
            }
            use slint::Model;
            let mut map: Vec<_> = ui.get_draft_map().iter().collect();
            let slot = ui.get_selected_slot() as usize;
            let mode = ui.get_keyboard_mode() as u8;
            if slot < 40 {
                for (pad, value) in map.iter_mut().enumerate() {
                    if pad == slot
                        || (pad < 32
                            && slot < 32
                            && protocol::representative(mode, pad)
                                == protocol::representative(mode, slot))
                    {
                        *value = if index == 0 { 0 } else { index + 3 };
                    }
                }
                ui.set_key_labels(labels(&map));
                ui.set_draft_map(slint::ModelRc::new(slint::VecModel::from(map)));
                map_dirty(&ui);
            }
        }
    });
    let weak = ui.as_weak();
    ui.on_select_mode(move |mode| {
        if let Some(ui) = weak
            .upgrade()
            .filter(|ui| ui.get_connected() && !ui.get_busy())
        {
            if !(0..6).contains(&mode) {
                return;
            }
            if let Ok(mut profiles) = draft_profiles(&ui) {
                profiles.mode = mode as u8;
                let map = profiles.mapping();
                set_profiles(&ui, profiles, false);
                ui.invoke_select_input(if ui.get_selected_slot() < 32 {
                    protocol::representative(map.mode, ui.get_selected_slot() as usize) as i32
                } else {
                    ui.get_selected_slot()
                });
                map_dirty(&ui);
            }
        }
    });
    let weak = ui.as_weak();
    ui.on_select_divider(move |divider| {
        if let Some(ui) = weak
            .upgrade()
            .filter(|ui| ui.get_connected() && !ui.get_busy() && ui.get_draft().keyboard)
            && (0..7).contains(&divider)
        {
            ui.set_divider_mode(divider);
            map_dirty(&ui);
        }
    });
    let weak = ui.as_weak();
    let dispatch = move |action: Result<Action, String>| {
        if let Some(ui) = weak.upgrade() {
            if ui.get_busy() {
                return;
            }
            let result = action.and_then(|action| send.try_send(action).map_err(|e| e.to_string()));
            match result {
                Ok(()) => {
                    ui.set_busy(true);
                    ui.set_error(false);
                    ui.set_status("Working…".into());
                }
                Err(error) => {
                    ui.set_error(true);
                    ui.set_status(error.into());
                }
            }
        }
    };
    let callback = dispatch.clone();
    ui.on_refresh(move || callback(Ok(Action::Discover)));
    let callback = dispatch.clone();
    let weak = ui.as_weak();
    ui.on_connect(move || {
        if let Some(ui) = weak.upgrade() {
            callback(Ok(Action::Connect(ui.get_port().to_string())));
        }
    });
    let callback = dispatch.clone();
    ui.on_disconnect(move || callback(Ok(Action::Disconnect)));
    let callback = dispatch.clone();
    ui.on_read(move || callback(Ok(Action::Read)));
    let callback = dispatch.clone();
    let weak = ui.as_weak();
    ui.on_apply(move |draft| {
        if let Some(ui) = weak.upgrade() {
            callback(
                config(draft).and_then(|config| Ok(Action::ApplyAll(config, draft_profiles(&ui)?))),
            );
        }
    });
    let callback = dispatch.clone();
    let weak = ui.as_weak();
    ui.on_apply_lighting(move |draft| {
        if let Some(ui) = weak.upgrade() {
            let applied = ui.get_applied();
            if let Ok(value) = config(lighting_settings(draft, applied.clone())) {
                if config(applied).ok() != Some(value) {
                    callback(Ok(Action::ApplyLighting(value)));
                }
            } else {
                callback(Err("invalid lighting settings".into()));
            }
        }
    });
    let callback = dispatch.clone();
    let weak = ui.as_weak();
    ui.on_apply_map(move || {
        if let Some(ui) = weak.upgrade() {
            callback(draft_profiles(&ui).map(Action::ApplyMap));
        }
    });
    let callback = dispatch.clone();
    let weak = ui.as_weak();
    ui.on_export_map(move || {
        if let Some(ui) = weak
            .upgrade()
            .filter(|ui| ui.get_connected() && !ui.get_busy())
        {
            callback(draft_profiles(&ui).map(Action::Export));
        }
    });
    let callback = dispatch.clone();
    let weak = ui.as_weak();
    ui.on_import_map(move || {
        if weak
            .upgrade()
            .is_some_and(|ui| ui.get_connected() && !ui.get_busy())
        {
            callback(Ok(Action::Import));
        }
    });
    let callback = dispatch.clone();
    let weak = ui.as_weak();
    ui.on_save(move |draft| {
        if let Some(ui) = weak.upgrade() {
            callback(
                config(draft).and_then(|config| Ok(Action::Save(config, draft_profiles(&ui)?))),
            );
        }
    });
    ui.on_reset(move || dispatch(Ok(Action::Reset)));
    let weak = ui.as_weak();
    let timer = slint::Timer::default();
    timer.start(
        slint::TimerMode::Repeated,
        std::time::Duration::from_millis(30),
        move || {
            if let (Some(ui), Ok(result)) = (weak.upgrade(), port_receive.try_recv())
                && !ui.get_connected()
                && !ui.get_busy()
            {
                match result {
                    Ok(ports) => {
                        use slint::Model;
                        let current: Vec<String> =
                            ui.get_ports().iter().map(|p| p.to_string()).collect();
                        if current != ports {
                            ui.set_port(selected_port(&ui.get_port(), &ports).into());
                            ui.set_ports(slint::ModelRc::new(slint::VecModel::from(
                                ports
                                    .into_iter()
                                    .map(slint::SharedString::from)
                                    .collect::<Vec<_>>(),
                            )));
                        }
                    }
                    Err(error) => {
                        ui.set_error(true);
                        ui.set_status(error.into());
                    }
                }
            }
            if let (Some(ui), Ok(input)) = (weak.upgrade(), preview_receive.try_recv())
                && ui.get_connected()
                && !ui.get_busy()
            {
                ui.set_input_levels(slint::ModelRc::new(slint::VecModel::from(
                    input_levels(input).to_vec(),
                )));
            }
            if let (Some(ui), Ok(update)) = (weak.upgrade(), receive.try_recv()) {
                if let Some(info) = update.info {
                    ui.set_device_info(info.into());
                }
                if let Some(ports) = update.ports {
                    ui.set_port(selected_port(&ui.get_port(), &ports).into());
                    ui.set_ports(slint::ModelRc::new(slint::VecModel::from(
                        ports
                            .into_iter()
                            .map(slint::SharedString::from)
                            .collect::<Vec<_>>(),
                    )));
                }
                if let Some(config) = update.config {
                    let applied = settings(config);
                    ui.set_draft(if update.lighting_only {
                        lighting_settings(applied.clone(), ui.get_draft())
                    } else {
                        applied.clone()
                    });
                    ui.set_applied(settings(config));
                }
                if let Some(map) = update.map {
                    set_profiles(&ui, map, true);
                    let map = map.mapping();
                    ui.set_applied_map(slint::ModelRc::new(slint::VecModel::from(
                        map.keys.into_iter().map(i32::from).collect::<Vec<_>>(),
                    )));
                    ui.set_applied_mode(i32::from(map.mode));
                    ui.set_applied_divider(i32::from(map.divider));
                    ui.set_map_dirty(false);
                    ui.invoke_select_input(ui.get_selected_slot());
                }
                if let Some(map) = update.import_map {
                    let profiles = match map {
                        keymap_file::Imported::All(profiles) => Ok(profiles),
                        keymap_file::Imported::Active(map) => {
                            draft_profiles(&ui).and_then(|mut profiles| {
                                profiles.put(map)?;
                                Ok(profiles)
                            })
                        }
                    };
                    if let Ok(profiles) = profiles {
                        set_profiles(&ui, profiles, false);
                    }
                    map_dirty(&ui);
                    ui.invoke_select_input(ui.get_selected_slot());
                }
                if !update.connected {
                    ui.set_device_info("".into());
                    ui.set_input_levels(slint::ModelRc::new(slint::VecModel::from(vec![0; 40])));
                }
                ui.set_connected(update.connected);
                ui.set_status(update.message.into());
                ui.set_error(update.error);
                if update.operation {
                    ui.set_busy(false);
                }
            }
        },
    );
    ui.invoke_refresh();
    Ok(timer)
}

fn lighting_settings(lighting: Settings, mut input: Settings) -> Settings {
    input.left = lighting.left;
    input.right = lighting.right;
    input.ground = lighting.ground;
    input.active = lighting.active;
    input.ground_brightness = lighting.ground_brightness;
    input.tower_brightness = lighting.tower_brightness;
    input.rainbow = lighting.rainbow;
    input
}

fn draft_map(ui: &AppWindow) -> Result<Mapping, String> {
    use slint::Model;
    let bytes = ui
        .get_draft_map()
        .iter()
        .map(|value| u8::try_from(value).map_err(|_| "invalid USB usage".to_string()))
        .collect::<Result<Vec<_>, _>>()?;
    Mapping {
        keys: protocol::keymap(&bytes)?,
        mode: u8::try_from(ui.get_keyboard_mode()).map_err(|_| "invalid keyboard mode")?,
        divider: u8::try_from(ui.get_divider_mode()).map_err(|_| "invalid divider")?,
    }
    .validate()
}

fn map_dirty(ui: &AppWindow) {
    ui.set_map_dirty(draft_profiles(ui).ok() != applied_profiles(ui).ok());
}

fn profile_model(profiles: Profiles) -> slint::ModelRc<i32> {
    slint::ModelRc::new(slint::VecModel::from(
        profiles
            .banks
            .into_iter()
            .flatten()
            .chain(profiles.shared)
            .map(i32::from)
            .collect::<Vec<_>>(),
    ))
}

fn set_profiles(ui: &AppWindow, profiles: Profiles, applied: bool) {
    ui.set_draft_profiles(profile_model(profiles));
    if applied {
        ui.set_applied_profiles(profile_model(profiles));
    }
    set_map(ui, profiles.mapping());
}

fn profiles_from_model(
    model: slint::ModelRc<i32>,
    mode: i32,
    divider: i32,
) -> Result<Profiles, String> {
    use slint::Model;
    let bytes = model
        .iter()
        .map(|v| u8::try_from(v).map_err(|_| "invalid profile usage"))
        .collect::<Result<Vec<_>, _>>()?;
    if bytes.len() != 200 {
        return Err("profiles not loaded; read RAM first".into());
    }
    Profiles {
        banks: std::array::from_fn(|mode| std::array::from_fn(|pad| bytes[mode * 32 + pad])),
        shared: bytes[192..].try_into().map_err(|_| "invalid shared keys")?,
        mode: u8::try_from(mode).map_err(|_| "invalid mode")?,
        divider: u8::try_from(divider).map_err(|_| "invalid divider")?,
    }
    .validate()
}

fn draft_profiles(ui: &AppWindow) -> Result<Profiles, String> {
    let mut profiles = profiles_from_model(
        ui.get_draft_profiles(),
        ui.get_keyboard_mode(),
        ui.get_divider_mode(),
    )?;
    profiles.put(draft_map(ui)?)?;
    Ok(profiles)
}

fn applied_profiles(ui: &AppWindow) -> Result<Profiles, String> {
    profiles_from_model(
        ui.get_applied_profiles(),
        ui.get_applied_mode(),
        ui.get_applied_divider(),
    )
}

fn set_map(ui: &AppWindow, map: Mapping) {
    let keys = map.keys.into_iter().map(i32::from).collect::<Vec<_>>();
    ui.set_keyboard_mode(i32::from(map.mode));
    ui.set_divider_mode(i32::from(map.divider));
    ui.set_key_labels(labels(&keys));
    ui.set_draft_map(slint::ModelRc::new(slint::VecModel::from(keys)));
    let mut starts = Vec::new();
    let mut widths = Vec::new();
    for cell in 0..16 {
        let pad = 30 - cell * 2;
        if protocol::representative(map.mode, pad) == pad {
            starts.push(pad as i32);
            widths.push(
                (0..16)
                    .filter(|&c| protocol::representative(map.mode, 30 - c * 2) == pad)
                    .count() as i32,
            );
        }
    }
    ui.set_group_starts(slint::ModelRc::new(slint::VecModel::from(starts)));
    ui.set_group_widths(slint::ModelRc::new(slint::VecModel::from(widths)));
}

fn labels(map: &[i32]) -> slint::ModelRc<slint::SharedString> {
    slint::ModelRc::new(slint::VecModel::from(
        map.iter()
            .map(|&usage| protocol::usage_label(usage as u8).into())
            .collect::<Vec<_>>(),
    ))
}

fn input_levels(input: [u8; 33]) -> [i32; 40] {
    std::array::from_fn(|slot| {
        if slot < 32 {
            i32::from(input[slot])
        } else if input[32] & (1 << (slot - 32)) != 0 {
            255
        } else {
            0
        }
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn hotplug_preserves_available_selection() {
        let ports = vec!["COM1".into(), "COM3".into()];
        assert_eq!(selected_port("COM3", &ports), "COM3");
        assert_eq!(selected_port("COM2", &ports), "COM1");
        assert_eq!(selected_port("COM3", &[]), "");
    }
    #[test]
    fn lighting_apply_preserves_input_edits() {
        let input = Settings {
            sensitivity: 16,
            keyboard: true,
            ..Default::default()
        };
        let lighting = Settings {
            left: 120,
            rainbow: true,
            ..Default::default()
        };
        let merged = lighting_settings(lighting, input);
        assert_eq!(merged.sensitivity, 16);
        assert!(merged.keyboard);
        assert_eq!(merged.left, 120);
        assert!(merged.rainbow);
    }
    #[test]
    fn native_input_order() {
        let mut input = [0; 33];
        input[0] = 17;
        input[31] = 255;
        input[32] = 0b1000_0011;
        let levels = input_levels(input);
        assert_eq!((levels[0], levels[31]), (17, 255));
        assert_eq!(&levels[32..], &[255, 255, 0, 0, 0, 0, 0, 255]);
    }
    #[test]
    fn ui_boundary_validation() {
        let valid = Settings {
            sensitivity: 1,
            ..Default::default()
        };
        assert!(config(valid.clone()).is_ok());
        for bad in [
            Settings {
                left: 360,
                ..valid.clone()
            },
            Settings {
                sensitivity: 17,
                ..valid.clone()
            },
            Settings {
                tower_brightness: -1,
                ..valid.clone()
            },
            Settings {
                ground_brightness: 256,
                ..valid
            },
        ] {
            assert!(config(bad).is_err());
        }
        let config = Config {
            keyboard: true,
            rainbow: true,
            sensitivity: 16,
            hues: [0, 120, 240, 359],
            brightness: [0, 255],
        };
        assert_eq!(super::config(settings(config)).unwrap(), config);
    }
}
