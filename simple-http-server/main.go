package main

import (
	"fmt"
	"log"
	"net"
	"strconv"
)

func main() {
	listen, err := net.Listen("tcp", ":8888")
	if err != nil {
		log.Fatal(err)
		return
	}

	defer listen.Close()

	for {
		conn, err := listen.Accept()
		if err != nil {
			log.Fatal(err)
		}

		go testConn(conn)
	}

}

func testConn(conn net.Conn) {
	defer conn.Close()
	var respBody = "Hello World"
	i := len(respBody)
	fmt.Println("响应的消息体长度：" + strconv.Itoa(i))

	var respHeader = "HTTP/1.1 200 OK\n" +
		"Content-Type: text/html;charset=utf-8\n" +
		"Content-Length: " + strconv.Itoa(i) + "\n"

	// header 头和　body 之间必须 \r\n
	resp := respHeader + "\r\n" + respBody
	fmt.Println(resp)

	n, _ := conn.Write([]byte(resp))
	fmt.Printf("发送的数据数量：%s\n", strconv.Itoa(n))

}
