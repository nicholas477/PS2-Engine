use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpListener;

use std::io;

#[tokio::main]
async fn main() -> io::Result<()> {
    let addr = "0.0.0.0:12345";

    let listener = TcpListener::bind(addr).await?;
    println!("TCP server listening on {}", addr);

    loop {
        // Asynchronously wait for an inbound connection
        let (mut socket, client_addr) = listener.accept().await?;
        println!("New client connected: {}", client_addr);

        // Spawn a background task to handle this connection concurrently
        tokio::spawn(async move {
            let mut buf = [0; 1024];

            // Loop to continuously read from the socket until EOF or error
            loop {
                match socket.read(&mut buf).await {
                    // Return value of Ok(0) signifies that the remote side closed the connection
                    Ok(0) => {
                        println!("Client {} disconnected cleanly", client_addr);
                        break;
                    }
                    Ok(n) => {
                        // Echo the exact data back to the socket
                        if let Err(e) = socket.write_all(&buf[0..n]).await {
                            eprintln!("Failed to write to socket: {}", e);
                            break;
                        }
                    }
                    Err(e) => {
                        eprintln!("Failed to read from socket: {}", e);
                        break;
                    }
                }
            }
        });
    }
}
