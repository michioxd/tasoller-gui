#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod config;
mod controller;
mod keymap_file;
mod protocol;
mod theme;
mod transport;

slint::include_modules!();

fn main() -> Result<(), Box<dyn std::error::Error>> {
    theme::select_backend()?;
    let ui = AppWindow::new()?;
    theme::setup(&ui);
    ui.set_app_version(env!("CARGO_PKG_VERSION").into());
    let _updates = controller::bind(&ui)?;
    match config::load_auto_lighting() {
        Ok(enabled) => ui.set_auto_lighting(enabled),
        Err(error) => {
            ui.set_error(true);
            ui.set_status(format!("Preference read failed: {error}").into());
        }
    }
    use slint::ComponentHandle;
    let weak = ui.as_weak();
    ui.on_github_clicked(move || {
        #[cfg(windows)]
        {
            let result = unsafe {
                windows_sys::Win32::UI::Shell::ShellExecuteW(
                    std::ptr::null_mut(),
                    windows_sys::core::w!("open"),
                    windows_sys::core::w!("https://github.com/michioxd/tasoller-gui"),
                    std::ptr::null(),
                    std::ptr::null(),
                    windows_sys::Win32::UI::WindowsAndMessaging::SW_SHOWNORMAL,
                )
            };
            if result as isize <= 32
                && let Some(ui) = weak.upgrade()
            {
                ui.set_error(true);
                ui.set_status("Unable to open GitHub in the default browser.".into());
            }
        }
    });
    let weak = ui.as_weak();
    ui.on_auto_lighting_changed(move |enabled| {
        if let Err(error) = config::save_auto_lighting(enabled)
            && let Some(ui) = weak.upgrade()
        {
            ui.set_error(true);
            ui.set_status(format!("Preference save failed: {error}").into());
        }
    });
    let weak = ui.as_weak();
    ui.window().on_close_requested(move || {
        if let Some(ui) = weak.upgrade().filter(|ui| ui.get_busy()) {
            ui.set_status("Wait for the current operation to finish before closing.".into());
            slint::CloseRequestResponse::KeepWindowShown
        } else {
            slint::CloseRequestResponse::HideWindow
        }
    });
    ui.run()?;
    Ok(())
}
