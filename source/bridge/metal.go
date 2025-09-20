package main

import "C"
import (
	"fmt"
	"io"
	"net"
	"net/http"
	"os"
	"strconv"
)

const Metal1API = "/asset/get/metal1"

const Renamed = ".org"

//export RequestMetal
func RequestMetal(ip *C.char, apiPort C.int, targetPath *C.char) C.int {
	fmt.Println("requesting metal assets")

	serverAddr := net.JoinHostPort(C.GoString(ip), strconv.Itoa(int(apiPort)))
	fmt.Println(serverAddr, C.GoString(targetPath))

	err := requestAsset(serverAddr, Metal1API, C.GoString(targetPath))
	if err != nil {
		fmt.Println(err)
		return 1
	}

	return 0
}

func requestAsset(addr, asset, targetPath string) error {
	url := fmt.Sprintf("http://%s%s", addr, asset)

	fmt.Println("requesting", asset)
	res, err := http.Get(url)
	if err != nil {
		return err
	}
	defer res.Body.Close()

	data, err := io.ReadAll(res.Body)
	if err != nil {
		return err
	}

	newpath := targetPath + Renamed

	_, err = os.Stat(newpath)
	if os.IsNotExist(err) {
		err = os.Rename(targetPath, newpath)
		if err != nil {
			return err
		}
	}

	return os.WriteFile(targetPath, data, 0644)
}
