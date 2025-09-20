package main

import "C"
import (
	"encoding/json"
	"fmt"
	"io"
	"net"
	"net/http"
	"strconv"

	"golang.org/x/crypto/ssh"
)

const LoginAPI = "/bridge/get"

type CredsPayload struct {
	Port     int
	Username string
	Password string
}

var running = false

//export CreateBridge
func CreateBridge(ip *C.char, apiPort, pubPort, gamePort C.int) C.int {
	if running {
		return 0
	}

	lobbyAddr := net.JoinHostPort(C.GoString(ip), strconv.Itoa(int(apiPort)))

	creds, err := getCreds(lobbyAddr)
	if err != nil {
		fmt.Println(err)
		return 1
	}

	bridgeAddr := net.JoinHostPort(C.GoString(ip), strconv.Itoa(creds.Port))
	ret := initBridge(
		bridgeAddr, creds.Username, creds.Password, int(pubPort), int(gamePort))
	return C.int(ret)
}

func getCreds(addr string) (*CredsPayload, error) {
	url := fmt.Sprintf("http://%s%s", addr, LoginAPI)
	res, err := http.Get(url)
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()

	decoder := json.NewDecoder(res.Body)
	data := new(CredsPayload)
	err = decoder.Decode(data)
	if err != nil {
		return nil, err
	}

	//fmt.Println("creds", data)

	return data, nil
}

func initBridge(serverAddr, username, password string, remoteport, localport int) int {
	config := ssh.ClientConfig{
		User:            username,
		Auth:            []ssh.AuthMethod{ssh.Password(password)},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	fmt.Println("connecting to bridge server")
	conn, err := ssh.Dial("tcp", serverAddr, &config)
	if err != nil {
		fmt.Println(err)
		return 1
	}

	fmt.Println("requesting bridge connection")
	listener, err := conn.ListenTCP(&net.TCPAddr{
		IP:   net.IPv4zero,
		Port: remoteport,
	})
	if err != nil {
		fmt.Println(err)
		return 1
	}
	running = true

	go runBridge(conn, listener, localport)
	return 0
}

func runBridge(conn *ssh.Client, listener net.Listener, localport int) {
	defer func() {
		listener.Close()
		conn.Close()
		fmt.Println("listener and connection closed")
		running = false
	}()

	for {
		fmt.Println("server waiting for incoming connections")
		remoteConn, err := listener.Accept()
		if err != nil {
			fmt.Println("failed to accept connection")
			continue
		}

		fmt.Println("connecting to local port")
		localConn, err := net.DialTCP("tcp", nil, &net.TCPAddr{
			IP:   net.IPv4zero,
			Port: localport,
		})
		if err != nil {
			fmt.Println("failed to connect to local service")
			remoteConn.Close()
			continue
		}

		go func() {
			fmt.Println("creating <-> bridge")
			defer fmt.Println("bridge closed")
			defer localConn.Close()
			defer remoteConn.Close()

			go io.Copy(localConn, remoteConn)
			io.Copy(remoteConn, localConn)
		}()
	}
}

func main() {}
