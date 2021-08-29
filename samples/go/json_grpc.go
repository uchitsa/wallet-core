package main

import (
	"fmt"
	"github.com/powerman/rpc-codec/jsonrpc2"
	"io"
	"net"
	"net/http"
	"net/rpc"
)

type HttpConn struct {
	in  io.Reader
	out io.Writer
}

func (c *HttpConn) Read(p []byte) (n int, err error) {
	return c.in.Read(p)
}

func (c *HttpConn) Write(d []byte) (n int, err error) {
	return c.out.Write(d)
}

func (c *HttpConn) Close() error {
	return nil
}

type Test struct{}

type HelloArgs struct {
	Name string
}

func (test *Test) Hello(args *HelloArgs, result *string) error {
	*result = "Hello " + args.Name
	return nil
}

func main() {

	server := rpc.NewServer()
	err := server.Register(&Test{})
	if err != nil {
		return
	}

	listener, err := net.Listen("tcp", ":2322")

	if err != nil {
		panic(err)
	}

	defer func(listener net.Listener) {
		err := listener.Close()
		if err != nil {
			fmt.Println("closing listener error")
		}
	}(listener)

	err = http.Serve(listener, http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.URL.Path == "/rpc" {
			serverCodec := jsonrpc2.NewServerCodec(&HttpConn{in: r.Body, out: w}, server)

			w.Header().Set("Content-type", "application/json")
			w.WriteHeader(200)

			if err1 := server.ServeRequest(serverCodec); err1 != nil {
				http.Error(w, "Error while serving JSON request", 500)
				return
			}
		} else {
			http.Error(w, "Unknown request", 404)
		}
	}))
	if err != nil {
		return
	}

	//TODO: dodelat zadanie
}
