#[cfg(windows)]
pub fn load_auto_lighting() -> std::io::Result<bool> {
    use windows_sys::Win32::System::Registry::*;
    let mut value = 0u32;
    let mut size = 4u32;
    let result = unsafe {
        RegGetValueW(
            HKEY_CURRENT_USER,
            windows_sys::core::w!("SOFTWARE\\TASOLLER"),
            windows_sys::core::w!("auto_lighting"),
            RRF_RT_REG_DWORD,
            std::ptr::null_mut(),
            (&mut value as *mut u32).cast(),
            &mut size,
        )
    };
    match result {
        0 => parse_auto_lighting(value),
        2 => Ok(false),
        error => Err(std::io::Error::from_raw_os_error(error as i32)),
    }
}

#[cfg(windows)]
pub fn save_auto_lighting(enabled: bool) -> std::io::Result<()> {
    use windows_sys::Win32::System::Registry::*;
    let mut key = std::ptr::null_mut();
    let value = u32::from(enabled);
    let result = unsafe {
        RegCreateKeyW(
            HKEY_CURRENT_USER,
            windows_sys::core::w!("SOFTWARE\\TASOLLER"),
            &mut key,
        )
    };
    if result != 0 {
        return Err(std::io::Error::from_raw_os_error(result as i32));
    }
    let result = unsafe {
        let result = RegSetKeyValueW(
            key,
            std::ptr::null(),
            windows_sys::core::w!("auto_lighting"),
            REG_DWORD,
            (&value as *const u32).cast(),
            4,
        );
        RegCloseKey(key);
        result
    };
    if result == 0 {
        Ok(())
    } else {
        Err(std::io::Error::from_raw_os_error(result as i32))
    }
}

#[cfg(not(windows))]
pub fn load_auto_lighting() -> std::io::Result<bool> {
    Ok(false)
}

#[cfg(not(windows))]
pub fn save_auto_lighting(_: bool) -> std::io::Result<()> {
    Err(std::io::Error::new(
        std::io::ErrorKind::Unsupported,
        "registry is only available on Windows",
    ))
}

#[cfg(any(windows, test))]
fn parse_auto_lighting(value: u32) -> std::io::Result<bool> {
    match value {
        0 => Ok(false),
        1 => Ok(true),
        _ => Err(std::io::Error::new(
            std::io::ErrorKind::InvalidData,
            "invalid auto_lighting DWORD",
        )),
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn validates_registry_boolean() {
        assert!(!super::parse_auto_lighting(0).unwrap());
        assert!(super::parse_auto_lighting(1).unwrap());
        assert!(super::parse_auto_lighting(2).is_err());
    }
}
