use crate::protocol::{self, Decoder};
use std::io::{Read, Write};
use std::time::{Duration, Instant};

pub fn discover() -> Result<Vec<String>, String> {
    let mut ports: Vec<_> = serialport::available_ports()
        .map_err(|e| e.to_string())?
        .into_iter()
        .filter_map(|port| match port.port_type {
            serialport::SerialPortType::UsbPort(usb) if usb.vid == 0x0ca3 && usb.pid == 0x0021 => {
                Some(port.port_name)
            }
            _ => None,
        })
        .collect();
    ports.sort();
    ports.dedup();
    Ok(ports)
}

pub fn open(name: &str) -> Result<Box<dyn serialport::SerialPort>, String> {
    if !discover()?.iter().any(|port| port == name) {
        return Err("port is no longer a compatible VID 0CA3 / PID 0021 device".into());
    }
    let port = serialport::new(name, 115_200)
        .timeout(Duration::from_millis(100))
        .flow_control(serialport::FlowControl::None)
        .open()
        .map_err(|e| e.to_string())?;
    port.clear(serialport::ClearBuffer::All)
        .map_err(|e| e.to_string())?;
    Ok(port)
}

pub fn exchange<T: Read + Write + ?Sized>(
    port: &mut T,
    command: u8,
    payload: &[u8],
) -> Result<Vec<u8>, String> {
    let wire = protocol::frame(command, payload)?;
    port.write_all(&wire)
        .map_err(|e| format!("serial write failed; outcome unknown: {e}"))?;
    port.flush()
        .map_err(|e| format!("serial flush failed; outcome unknown: {e}"))?;
    let deadline = Instant::now() + Duration::from_secs(2);
    let mut decoder = Decoder::default();
    let mut buffer = [0; 256];
    let mut received = 0;
    while Instant::now() < deadline {
        let count = match port.read(&mut buffer) {
            Ok(0) => return Err("serial connection closed; outcome unknown".into()),
            Ok(count) => count,
            Err(e)
                if matches!(
                    e.kind(),
                    std::io::ErrorKind::TimedOut
                        | std::io::ErrorKind::WouldBlock
                        | std::io::ErrorKind::Interrupted
                ) =>
            {
                continue;
            }
            Err(e) => return Err(format!("serial read failed; outcome unknown: {e}")),
        };
        received += count;
        if received > 16_384 {
            return Err("excessive serial traffic; outcome unknown".into());
        }
        for &byte in &buffer[..count] {
            if let Some((reply, data)) = decoder.push(byte)? {
                if reply == command {
                    protocol::response(command, &data)?;
                    return Ok(data);
                }
                if !matches!(reply, 0x01 | 0x06 | 0x0b) {
                    return Err(format!("unexpected response {reply:02X}; outcome unknown"));
                }
            }
        }
    }
    Err("response timed out; outcome unknown (not retried)".into())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;
    struct Script {
        input: Cursor<Vec<u8>>,
        written: Vec<u8>,
    }
    impl Read for Script {
        fn read(&mut self, bytes: &mut [u8]) -> std::io::Result<usize> {
            self.input.read(&mut bytes[..1])
        }
    }
    impl Write for Script {
        fn write(&mut self, bytes: &[u8]) -> std::io::Result<usize> {
            self.written.extend(bytes);
            Ok(bytes.len())
        }
        fn flush(&mut self) -> std::io::Result<()> {
            Ok(())
        }
    }
    #[test]
    fn matching_streams_errors_and_no_write_retry() {
        let mut input = protocol::frame(1, &[0; 32]).unwrap();
        input.extend(protocol::frame(4, &[]).unwrap());
        let mut port = Script {
            input: Cursor::new(input),
            written: vec![],
        };
        assert_eq!(exchange(&mut port, 4, &[]).unwrap(), []);
        assert_eq!(port.written, protocol::frame(4, &[]).unwrap());
        for input in [
            protocol::frame(0xf5, &[1, 5]).unwrap(),
            protocol::frame(0xf4, &[1, 0]).unwrap(),
            vec![],
        ] {
            let mut port = Script {
                input: Cursor::new(input),
                written: vec![],
            };
            assert!(exchange(&mut port, 0xf5, &[1]).is_err());
            assert_eq!(port.written, protocol::frame(0xf5, &[1]).unwrap());
        }
    }
}
