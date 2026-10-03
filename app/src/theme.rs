pub fn select_backend() -> Result<(), slint::PlatformError> {
    #[cfg(windows)]
    return slint::BackendSelector::new()
        .backend_name("winit".into())
        .with_winit_window_attributes_hook(|attributes| attributes.with_transparent(true))
        .select();
    #[cfg(not(windows))]
    Ok(())
}

pub fn setup(ui: &crate::AppWindow) {
    #[cfg(windows)]
    {
        use slint::ComponentHandle;
        use slint::winit_030::winit::{event::WindowEvent, window::Theme};
        use slint::winit_030::{EventResult, WinitWindowAccessor};

        let weak = ui.as_weak();
        if let Err(error) = slint::spawn_local(async move {
            let Some(ui) = weak.upgrade() else { return };
            let window = ui.window();
            let Ok(native) = window.winit_window().await else {
                return;
            };
            ui.set_backdrop_enabled(apply(window, native.theme() == Some(Theme::Dark)));
            if let Err(error) = set_window_icon(window) {
                eprintln!("unable to set window icon: {error}");
            }
            let weak = ui.as_weak();
            window.on_winit_window_event(move |window, event| {
                if let WindowEvent::ThemeChanged(theme) = event
                    && let Some(ui) = weak.upgrade()
                {
                    ui.set_backdrop_enabled(apply(window, *theme == Theme::Dark));
                }
                EventResult::Propagate
            });
        }) {
            eprintln!("unable to initialize window backdrop: {error}");
        }
    }
    #[cfg(not(windows))]
    let _ = ui;
}

#[cfg(windows)]
fn set_window_icon(window: &slint::Window) -> std::io::Result<()> {
    use raw_window_handle::{HasWindowHandle, RawWindowHandle};
    use windows_sys::Win32::{System::LibraryLoader::GetModuleHandleW, UI::WindowsAndMessaging::*};
    let provider = window.window_handle();
    let handle = provider.window_handle().map_err(std::io::Error::other)?;
    let RawWindowHandle::Win32(handle) = handle.as_raw() else {
        return Err(std::io::Error::other("not a Windows window"));
    };
    let module = unsafe { GetModuleHandleW(std::ptr::null()) };
    if module.is_null() {
        return Err(std::io::Error::last_os_error());
    }
    for (kind, size) in [(ICON_SMALL, 16), (ICON_BIG, 32)] {
        let icon = unsafe {
            LoadImageW(
                module,
                1usize as *const u16,
                IMAGE_ICON,
                size,
                size,
                LR_SHARED,
            )
        };
        if icon.is_null() {
            return Err(std::io::Error::last_os_error());
        }
        unsafe {
            SendMessageW(
                handle.hwnd.get() as _,
                WM_SETICON,
                kind as usize,
                icon as isize,
            );
        }
    }
    Ok(())
}

#[cfg(windows)]
fn apply(window: &slint::Window, dark: bool) -> bool {
    use raw_window_handle::{HasWindowHandle, RawWindowHandle};
    use std::ffi::{c_char, c_void};

    #[repr(C)]
    struct AccentPolicy {
        state: u32,
        flags: u32,
        tint: u32,
        animation: u32,
    }
    #[repr(C)]
    struct AttributeData {
        attribute: u32,
        data: *mut c_void,
        size: usize,
    }
    #[repr(C)]
    struct Margins {
        left: i32,
        right: i32,
        top: i32,
        bottom: i32,
    }
    type SetAttribute = unsafe extern "system" fn(isize, *mut AttributeData) -> i32;

    #[link(name = "kernel32")]
    unsafe extern "system" {
        fn GetModuleHandleW(name: *const u16) -> isize;
        fn GetProcAddress(
            module: isize,
            name: *const c_char,
        ) -> Option<unsafe extern "system" fn()>;
    }
    #[link(name = "dwmapi")]
    unsafe extern "system" {
        fn DwmSetWindowAttribute(
            hwnd: isize,
            attribute: u32,
            value: *const c_void,
            size: u32,
        ) -> i32;
        fn DwmExtendFrameIntoClientArea(hwnd: isize, margins: *const Margins) -> i32;
    }

    let provider = window.window_handle();
    let Ok(handle) = provider.window_handle() else {
        return false;
    };
    let RawWindowHandle::Win32(handle) = handle.as_raw() else {
        return false;
    };
    let hwnd = handle.hwnd.get();
    let library: Vec<u16> = "user32.dll\0".encode_utf16().collect();
    let function = unsafe {
        let module = GetModuleHandleW(library.as_ptr());
        if module == 0 {
            return false;
        }
        GetProcAddress(module, c"SetWindowCompositionAttribute".as_ptr())
    };
    let Some(function) = function else {
        return false;
    };
    let set_attribute: SetAttribute = unsafe { std::mem::transmute(function) };
    let mut policy = AccentPolicy {
        state: 4,
        flags: 0,
        tint: acrylic_tint(dark),
        animation: 0,
    };
    let mut attribute = AttributeData {
        attribute: 19,
        data: (&mut policy as *mut AccentPolicy).cast(),
        size: size_of::<AccentPolicy>(),
    };
    let dark_mode = i32::from(dark);
    let margins = Margins {
        left: -1,
        right: -1,
        top: -1,
        bottom: -1,
    };
    unsafe {
        let _ = DwmSetWindowAttribute(hwnd, 20, (&dark_mode as *const i32).cast(), 4);
        if set_attribute(hwnd, &mut attribute) == 0 {
            return false;
        }
        if DwmExtendFrameIntoClientArea(hwnd, &margins) < 0 {
            policy.state = 0;
            attribute.data = (&mut policy as *mut AccentPolicy).cast();
            let _ = set_attribute(hwnd, &mut attribute);
            return false;
        }
        let caption_color = 0xFFFF_FFFEu32;
        let _ = DwmSetWindowAttribute(hwnd, 35, (&caption_color as *const u32).cast(), 4);
    }
    true
}

#[cfg(any(windows, test))]
fn acrylic_tint(dark: bool) -> u32 {
    if dark { 0xCC20_2020 } else { 0xCCF7_F7F7 }
}

#[cfg(test)]
mod tests {
    #[test]
    fn acrylic_tints_have_translucent_alpha() {
        for dark in [false, true] {
            let tint = super::acrylic_tint(dark);
            assert_eq!(tint >> 24, 0xCC);
            assert_eq!(tint & 0xFF, (tint >> 16) & 0xFF);
        }
        assert_ne!(super::acrylic_tint(false), super::acrylic_tint(true));
    }
}
