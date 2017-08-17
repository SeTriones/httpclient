package main

import (
	"crypto/md5"
	"io/ioutil"
	"log"
	"net/http"
)

func test(w http.ResponseWriter, r *http.Request) {
	r.ParseForm()
	r.ParseMultipartForm(int64(1024 * 1024 * 100))
	a := r.FormValue("a")
	b := r.FormValue("b")
	log.Printf("a=%s,b=%s\n", a, b)
	file, header, err := r.FormFile("file1")
	if err != nil {
		log.Printf("get form file err=%v\n", err)
		w.WriteHeader(400)
		w.Write([]byte("{\"msg\"=\"fail\"}"))
	}
	fileContent, err := ioutil.ReadAll(file)
	if err != nil {
		log.Printf("read form file err=%v\n", err)
		w.WriteHeader(500)
		w.Write([]byte("{\"msg\"=\"fail\"}"))
	}
	log.Printf("filename=%s\n", header.Filename)
	log.Printf("file md5=%x\n", md5.Sum(fileContent))
	w.WriteHeader(200)
	w.Write([]byte("{\"msg\"=\"succ\"}"))
}

func main() {
	mux := http.NewServeMux()
	mux.HandleFunc("/hello", test)
	http.ListenAndServe(":11086", mux)
}
