pub const MAX_PAYLOAD: usize = 97;
pub const GET_INFO: u8 = 0xf2;
pub const GET_CONFIG: u8 = 0xf3;
pub const SET_CONFIG: u8 = 0xf4;
pub const SAVE_CONFIG: u8 = 0xf5;
pub const RESET_CONFIG: u8 = 0xf6;
pub const GET_KEYMAP: u8 = 0xf7;
pub const SET_KEYMAP: u8 = 0xf8;
pub const GET_INPUT: u8 = 0xf9;
pub const GET_MODES: u8 = 0xfa;
pub const GET_PROFILE: u8 = 0xfb;
pub const SET_PROFILE: u8 = 0xfc;
pub const REPORT_DISABLE: u8 = 0x04;

pub type Keymap = [u8; 40];

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Profiles {
    pub banks: [[u8; 32]; 6],
    pub shared: [u8; 8],
    pub mode: u8,
    pub divider: u8,
}

impl Profiles {
    pub fn mapping(self) -> Mapping {
        let mut keys = [0; 40];
        keys[..32].copy_from_slice(&self.banks[usize::from(self.mode)]);
        keys[32..].copy_from_slice(&self.shared);
        Mapping {
            keys,
            mode: self.mode,
            divider: self.divider,
        }
    }

    pub fn put(&mut self, map: Mapping) -> Result<(), String> {
        map.validate()?;
        self.banks[usize::from(map.mode)].copy_from_slice(&map.keys[..32]);
        self.shared.copy_from_slice(&map.keys[32..]);
        self.mode = map.mode;
        self.divider = map.divider;
        Ok(())
    }

    pub fn validate(self) -> Result<Self, String> {
        if self.mode > 5 || self.divider > 6 {
            return Err("invalid profile mode or divider".into());
        }
        for mode in 0..6 {
            Self { mode, ..self }.mapping().validate()?;
        }
        Ok(self)
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Mapping {
    pub keys: Keymap,
    pub mode: u8,
    pub divider: u8,
}

pub fn representative(mode: u8, pad: usize) -> usize {
    if mode == 0 {
        return pad;
    }
    let cell = (31 - pad) / 2;
    let start = if mode == 2 {
        match cell {
            0 | 15 => cell,
            _ => 1 + (cell - 1) / 2 * 2,
        }
    } else {
        let width = match mode {
            1 => 1,
            3 => 2,
            4 => 4,
            _ => 8,
        };
        cell / width * width
    };
    30 - 2 * start
}

impl Mapping {
    pub fn validate(self) -> Result<Self, String> {
        keymap(&self.keys)?;
        if self.mode > 5 || self.divider > 6 {
            return Err("invalid keyboard mode or divider".into());
        }
        if (0..32).any(|pad| self.keys[pad] != self.keys[representative(self.mode, pad)]) {
            return Err("all pads in a keyboard group must share one usage".into());
        }
        Ok(self)
    }

    #[cfg(test)]
    pub fn normalize(&mut self) {
        let previous = self.keys;
        for pad in 0..32 {
            self.keys[pad] = previous[representative(self.mode, pad)];
        }
    }

    pub fn request(self) -> Result<Vec<u8>, String> {
        self.validate()?;
        let mut payload = vec![1, self.mode, self.divider];
        payload.extend(self.keys);
        Ok(payload)
    }
}

pub fn keymap(bytes: &[u8]) -> Result<Keymap, String> {
    let map: Keymap = bytes
        .try_into()
        .map_err(|_| "keymap must contain 40 usages")?;
    if map
        .iter()
        .any(|&usage| usage != 0 && !(4..=0xa4).contains(&usage))
    {
        return Err("only Unassigned or ordinary USB usages 04-A4 are supported".into());
    }
    Ok(map)
}

pub fn info(data: &[u8]) -> Result<String, String> {
    if data.len() != 16
        || data[0] < 3
        || data[1] != 1
        || u32::from_le_bytes(data[4..8].try_into().map_err(|_| "invalid capabilities")?) & 127
            != 127
        || &data[8..] != b"TAS-HOST"
    {
        return Err(
            "incompatible device: requires firmware major 1, API 1.3+, capabilities 127, TAS-HOST"
                .into(),
        );
    }
    Ok(format!(
        "TAS-HOST {}.{}.{} - API 1.{} - capabilities {}",
        data[1],
        data[2],
        data[3],
        data[0],
        u32::from_le_bytes(data[4..8].try_into().map_err(|_| "invalid capabilities")?)
    ))
}

pub fn usage_label(usage: u8) -> String {
    match usage {
        0 => "Unassigned".into(),
        4..=29 => char::from(b'A' + usage - 4).to_string(),
        30..=38 => (usage - 29).to_string(),
        39 => "0".into(),
        40..=56 => [
            "Enter",
            "Escape",
            "Backspace",
            "Tab",
            "Space",
            "-",
            "=",
            "[",
            "]",
            "\\",
            "Non-US #",
            ";",
            "'",
            "Grave",
            ",",
            ".",
            "/",
        ][usize::from(usage - 40)]
        .into(),
        57 => "Caps Lock".into(),
        58..=69 => format!("F{}", usage - 57),
        70..=83 => [
            "Print Screen",
            "Scroll Lock",
            "Pause",
            "Insert",
            "Home",
            "Page Up",
            "Delete",
            "End",
            "Page Down",
            "Right",
            "Left",
            "Down",
            "Up",
            "Num Lock",
        ][usize::from(usage - 70)]
        .into(),
        84..=99 => [
            "Keypad /",
            "Keypad *",
            "Keypad -",
            "Keypad +",
            "Keypad Enter",
            "Keypad 1",
            "Keypad 2",
            "Keypad 3",
            "Keypad 4",
            "Keypad 5",
            "Keypad 6",
            "Keypad 7",
            "Keypad 8",
            "Keypad 9",
            "Keypad 0",
            "Keypad .",
        ][usize::from(usage - 84)]
        .into(),
        100 => "Non-US \\".into(),
        101 => "Application".into(),
        102 => "Power".into(),
        103 => "Keypad =".into(),
        104..=115 => format!("F{}", usage - 91),
        _ => format!("USB 0x{usage:02X}"),
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Config {
    pub keyboard: bool,
    pub rainbow: bool,
    pub sensitivity: u8,
    pub hues: [u16; 4],
    pub brightness: [u8; 2],
}

impl Config {
    pub fn encode(self) -> Result<[u8; 14], String> {
        if !(1..=16).contains(&self.sensitivity) || self.hues.iter().any(|h| *h > 359) {
            return Err("sensitivity must be 1-16; hues must be 0-359".into());
        }
        let mut bytes = [0; 14];
        bytes[0] = 1;
        bytes[1] = u8::from(self.keyboard) | (u8::from(self.rainbow) << 1);
        bytes[2] = self.sensitivity;
        for (i, hue) in self.hues.iter().enumerate() {
            bytes[4 + i * 2..6 + i * 2].copy_from_slice(&hue.to_le_bytes());
        }
        bytes[12..].copy_from_slice(&self.brightness);
        Ok(bytes)
    }

    pub fn decode(bytes: &[u8]) -> Result<Self, String> {
        if bytes.len() != 14 || bytes[0] != 1 || bytes[1] & !3 != 0 || bytes[3] != 0 {
            return Err("invalid config length, layout, flags or reserved byte".into());
        }
        let config = Self {
            keyboard: bytes[1] & 1 != 0,
            rainbow: bytes[1] & 2 != 0,
            sensitivity: bytes[2],
            hues: std::array::from_fn(|i| u16::from_le_bytes([bytes[4 + i * 2], bytes[5 + i * 2]])),
            brightness: [bytes[12], bytes[13]],
        };
        config.encode()?;
        Ok(config)
    }
}

pub fn request(command: u8, config: Option<Config>) -> Result<Vec<u8>, String> {
    match (command, config) {
        (SET_CONFIG, Some(config)) => {
            let mut payload = vec![1];
            payload.extend(config.encode()?);
            Ok(payload)
        }
        (
            GET_INFO | GET_CONFIG | SAVE_CONFIG | RESET_CONFIG | GET_KEYMAP | GET_INPUT | GET_MODES,
            None,
        ) => Ok(vec![1]),
        (REPORT_DISABLE, None) => Ok(vec![]),
        _ => Err("invalid command/config combination".into()),
    }
}

pub fn response(command: u8, payload: &[u8]) -> Result<&[u8], String> {
    if command == REPORT_DISABLE {
        return if payload.is_empty() {
            Ok(payload)
        } else {
            Err("invalid report-disable response".into())
        };
    }
    if payload.len() < 2 || payload[0] != 1 {
        return Err("invalid response header".into());
    }
    if payload[1] != 0 {
        if payload.len() != 2 {
            return Err("error response contains unexpected data".into());
        }
        return Err(match payload[1] {
            1 => "device: bad length",
            2 => "device: unsupported API version",
            3 => "device: invalid value",
            4 => "device: busy (not retried)",
            5 => "device: storage failure; settings were NOT confirmed saved",
            _ => "device: unknown status",
        }
        .into());
    }
    let data = &payload[2..];
    match command {
        GET_INFO => {
            info(data)?;
            Ok(data)
        }
        GET_KEYMAP => {
            keymap(data)?;
            Ok(data)
        }
        GET_PROFILE if data.len() == 32 => {
            let mut keys = [0; 40];
            keys[..32].copy_from_slice(data);
            keymap(&keys)?;
            Ok(data)
        }
        GET_INPUT if data.len() == 33 => Ok(data),
        GET_MODES if data.len() == 2 && data[0] <= 5 && data[1] <= 6 => Ok(data),
        GET_CONFIG => {
            Config::decode(data)?;
            Ok(data)
        }
        SET_CONFIG | SAVE_CONFIG | RESET_CONFIG | SET_KEYMAP | SET_PROFILE if data.is_empty() => {
            Ok(data)
        }
        _ => Err("incompatible or malformed response".into()),
    }
}

pub fn frame(command: u8, payload: &[u8]) -> Result<Vec<u8>, String> {
    let length = u8::try_from(payload.len()).map_err(|_| "payload too large")?;
    if payload.len() > MAX_PAYLOAD {
        return Err("payload too large".into());
    }
    let mut body = vec![command, length];
    body.extend_from_slice(payload);
    let sum = body.iter().fold(0xffu8, |sum, b| sum.wrapping_add(*b));
    body.push(0u8.wrapping_sub(sum));
    let mut wire = Vec::with_capacity(1 + body.len() * 2);
    wire.push(0xff);
    for b in body {
        match b {
            0xff => wire.extend([0xfd, 0xfe]),
            0xfd => wire.extend([0xfd, 0xfc]),
            _ => wire.push(b),
        }
    }
    Ok(wire)
}

#[derive(Default)]
pub struct Decoder {
    body: Vec<u8>,
    active: bool,
    escape: bool,
}

impl Decoder {
    pub fn push(&mut self, byte: u8) -> Result<Option<(u8, Vec<u8>)>, String> {
        if byte == 0xff {
            self.body.clear();
            self.active = true;
            self.escape = false;
            return Ok(None);
        }
        if !self.active {
            return Ok(None);
        }
        let byte = if self.escape {
            self.escape = false;
            match byte {
                0xfe => 0xff,
                0xfc => 0xfd,
                _ => {
                    self.active = false;
                    self.body.clear();
                    return Err("invalid escape".into());
                }
            }
        } else if byte == 0xfd {
            self.escape = true;
            return Ok(None);
        } else {
            byte
        };
        self.body.push(byte);
        if self.body.len() == 2 && usize::from(self.body[1]) > MAX_PAYLOAD {
            self.active = false;
            self.body.clear();
            return Err("frame exceeds 97-byte payload limit".into());
        }
        if self.body.len() >= 3 && self.body.len() == usize::from(self.body[1]) + 3 {
            self.active = false;
            if self.body.iter().fold(0xffu8, |sum, b| sum.wrapping_add(*b)) != 0 {
                self.body.clear();
                return Err("checksum mismatch".into());
            }
            let packet = (self.body[0], self.body[2..self.body.len() - 1].to_vec());
            self.body.clear();
            return Ok(Some(packet));
        }
        Ok(None)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn independent_profile_drafts_roundtrip() {
        let mut profiles = Profiles {
            banks: [[4; 32]; 6],
            shared: [58; 8],
            mode: 4,
            divider: 4,
        };
        let mut map = profiles.mapping();
        for pad in 0..32 {
            map.keys[pad] = [4, 7, 11, 12][(31 - pad) / 8];
        }
        profiles.put(map).unwrap();
        for mode in 0..6 {
            profiles.mode = mode;
            profiles.validate().unwrap();
            if mode != 4 {
                let mut other = profiles.mapping();
                other.keys[..32].fill(30 + mode);
                profiles.put(other).unwrap();
            }
            profiles.mode = 4;
            assert_eq!(profiles.mapping(), map);
        }
        let before = profiles;
        let mut invalid = profiles.mapping();
        invalid.keys[0] = 5;
        assert!(profiles.put(invalid).is_err());
        assert_eq!(profiles, before);
        let mut invalid = profiles;
        invalid.mode = 6;
        assert!(invalid.validate().is_err());
    }

    #[test]
    fn exact_grouping_and_mode_validation() {
        let starts = [
            [0; 16],
            [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
            [0, 1, 1, 3, 3, 5, 5, 7, 7, 9, 9, 11, 11, 13, 13, 15],
            [0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14],
            [0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12],
            [0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8],
        ];
        let original = std::array::from_fn(|i| i as u8 + 4);
        for mode in 0..6 {
            let mut map = Mapping {
                keys: original,
                mode,
                divider: 6,
            };
            map.normalize();
            for pad in 0..32 {
                let expected = if mode == 0 {
                    pad
                } else {
                    30 - 2 * starts[mode as usize][(31 - pad) / 2]
                };
                assert_eq!(representative(mode, pad), expected);
                assert_eq!(map.keys[pad], original[expected]);
            }
            assert_eq!(map.keys[32..], original[32..]);
            assert!(map.validate().is_ok());
            let previous = map;
            map.normalize();
            assert_eq!(map, previous);
            if mode != 0 {
                map.keys[1] += 1;
                assert!(map.request().is_err());
            }
        }
        for (mode, divider) in [(6, 0), (0, 7), (255, 255)] {
            assert!(
                Mapping {
                    keys: [4; 40],
                    mode,
                    divider
                }
                .request()
                .is_err()
            );
            assert!(response(GET_MODES, &[1, 0, mode, divider]).is_err());
        }
        assert!(response(GET_MODES, &[1, 0, 5, 6]).is_ok());
        assert!(response(GET_MODES, &[1, 0, 5]).is_err());
        assert!(response(GET_MODES, &[1, 0, 5, 6, 0]).is_err());
    }

    pub fn config() -> Config {
        Config {
            keyboard: true,
            rainbow: false,
            sensitivity: 16,
            hues: [0, 359, 253, 255],
            brightness: [0, 255],
        }
    }

    #[test]
    fn framing_roundtrip_and_boundaries() {
        for size in 0..=97 {
            let payload: Vec<_> = (0..size).map(|i| [0xff, 0xfd, 0, 1][i % 4]).collect();
            let mut decoder = Decoder::default();
            let packets: Vec<_> = frame(0xfd, &payload)
                .unwrap()
                .into_iter()
                .filter_map(|b| decoder.push(b).unwrap())
                .collect();
            assert_eq!(packets, vec![(0xfd, payload)]);
        }
        assert!(frame(1, &[0; 98]).is_err());
        assert_eq!(frame(REPORT_DISABLE, &[]).unwrap(), [255, 4, 0, 253, 252]);
    }

    #[test]
    fn malformed_and_resynchronization() {
        let mut decoder = Decoder::default();
        for b in [255, 1, 98] {
            let _ = decoder.push(b);
        }
        assert!(decoder.body.is_empty());
        for b in [255, 1, 0] {
            decoder.push(b).unwrap();
        }
        assert!(decoder.push(1).is_err());
        for b in [255, 253] {
            decoder.push(b).unwrap();
        }
        assert!(decoder.push(1).is_err());
        for b in [255, 1, 3, 253] {
            decoder.push(b).unwrap();
        }
        let packets: Vec<_> = frame(2, &[3])
            .unwrap()
            .into_iter()
            .filter_map(|b| decoder.push(b).unwrap())
            .collect();
        assert_eq!(packets, vec![(2, vec![3])]);
        for b in 0..=255 {
            let _ = decoder.push(b);
            assert!(decoder.body.len() <= 100);
        }
    }

    #[test]
    fn config_validation() {
        let bytes = config().encode().unwrap();
        assert_eq!(Config::decode(&bytes).unwrap(), config());
        for (index, value) in [(0, 2), (1, 4), (2, 0), (2, 17), (3, 1), (5, 255)] {
            let mut bad = bytes;
            bad[index] = value;
            assert!(Config::decode(&bad).is_err());
        }
        assert!(Config::decode(&bytes[..13]).is_err());
    }

    #[test]
    fn requests_and_responses() {
        for command in [GET_INFO, GET_CONFIG, SAVE_CONFIG, RESET_CONFIG] {
            assert_eq!(request(command, None).unwrap(), [1]);
        }
        assert_eq!(request(SET_CONFIG, Some(config())).unwrap().len(), 15);
        assert!(request(SET_CONFIG, None).is_err());
        assert!(request(GET_INFO, Some(config())).is_err());
        let info = [
            1, 0, 3, 1, 3, 0, 127, 0, 0, 0, b'T', b'A', b'S', b'-', b'H', b'O', b'S', b'T',
        ];
        assert!(response(GET_INFO, &info).is_ok());
        for i in [3, 6, 10, 17] {
            let mut bad = info;
            bad[i] ^= 1;
            assert!(response(GET_INFO, &bad).is_err());
        }
        let mut newer = info;
        newer[2] = 9;
        newer[4] = 7;
        newer[5] = 8;
        newer[6] = 127;
        assert!(response(GET_INFO, &newer).is_ok());
        for len in 0..info.len() {
            assert!(response(GET_INFO, &info[..len]).is_err());
        }
        let mut config_response = vec![1, 0];
        config_response.extend(config().encode().unwrap());
        assert!(response(GET_CONFIG, &config_response).is_ok());
        for status in 1..=255 {
            assert!(response(SAVE_CONFIG, &[1, status]).is_err());
        }
        assert!(response(SAVE_CONFIG, &[1, 0]).is_ok());
        for bytes in [
            &[][..],
            &[1][..],
            &[2, 0][..],
            &[1, 0, 0][..],
            &[1, 5, 0][..],
        ] {
            assert!(response(SAVE_CONFIG, bytes).is_err());
        }
        assert!(response(REPORT_DISABLE, &[]).is_ok());
        assert!(response(REPORT_DISABLE, &[1]).is_err());
    }

    #[test]
    fn maps_and_live_input() {
        for usage in 0..=255 {
            assert_eq!(
                keymap(&[usage; 40]).is_ok(),
                usage == 0 || (4..=0xa4).contains(&usage)
            );
        }
        assert!(keymap(&[4; 39]).is_err());
        assert_eq!(
            Mapping {
                keys: [4; 40],
                mode: 0,
                divider: 4
            }
            .request()
            .unwrap()
            .len(),
            43
        );
        let mut reply = vec![1, 0];
        reply.extend([4; 40]);
        assert!(response(GET_KEYMAP, &reply).is_ok());
        for len in 0..reply.len() {
            assert!(response(GET_KEYMAP, &reply[..len]).is_err());
        }
        reply[2] = 0xe0;
        assert!(response(GET_KEYMAP, &reply).is_err());
        assert!(response(SET_KEYMAP, &[1, 0]).is_ok());
        assert!(response(SET_KEYMAP, &[1, 0, 0]).is_err());
        let input = [1, 0].into_iter().chain([255; 33]).collect::<Vec<_>>();
        assert!(response(GET_INPUT, &input).is_ok());
        for len in 0..input.len() {
            assert!(response(GET_INPUT, &input[..len]).is_err());
        }
        assert!(response(GET_INPUT, &[1, 3]).is_err());
    }
}
