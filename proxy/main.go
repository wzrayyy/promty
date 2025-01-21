package main

import (
	"bytes"
	"compress/gzip"
	"fmt"
	"io"
	"net/http"
	"net/http/httputil"
	"net/url"
	"os"
	"regexp"
	"strings"
)

func main() {
	proxy := &httputil.ReverseProxy{
		Rewrite: func(r *httputil.ProxyRequest) {
			host, _ := Yadro2Kernel(r.In.Host, true)
			url, _ := url.Parse("https://" + host)

			r.SetURL(url)

			// We only support gzip decoding
			r.Out.Header.Set("Accept-Encoding", "gzip")
		},
		ModifyResponse: func(r *http.Response) error {
			// Disable security policy because of the domain restrictions.
			r.Header.Del("Content-Security-Policy")

			// Skip non-html pages.
			contentType := r.Header.Get("Content-Type")
			if !strings.HasPrefix(contentType, "text/") {
				return nil
			}
			r.Header.Set("Content-Type", contentType+";charset=utf-8")

			// Read response body into data. If body is encoded, decode it.
			var data []byte

			switch r.Header.Get("Content-Encoding") {
			case "gzip":
				reader, _ := gzip.NewReader(r.Body)
				data, _ = io.ReadAll(reader)
				r.Body.Close()
			default:
				data, _ = io.ReadAll(r.Body)
				r.Body.Close()
			}

			// Rewrite 30x redirect location
			locHeader := r.Header.Get("Location")
			if locHeader != "" {
				re := regexp.MustCompile(`https?:\/\/[A-Za-z\-\.]*.?kernel\.org\/`)
				r.Header.Set("Location", string(re.ReplaceAllFunc([]byte(locHeader), func(original_raw []byte) []byte {
					kernel := strings.ToLower(string(original_raw))

					kernel = strings.TrimPrefix(kernel, "https://")
					kernel = strings.TrimPrefix(kernel, "http://")
					kernel = strings.TrimPrefix(kernel, "www.")
					kernel = strings.TrimSuffix(kernel, "/")

					yadro, exists := Kernel2Yadro(kernel)

					if !exists {
						fmt.Println("Missing:", kernel)
						return original_raw
					}

					return []byte("http://" + yadro + "/") // TODO: https
				})))
			}

			// Modify the response.
			data = modifyResponse(data)

			// Remove headers that mess with body encoding and set the body.
			r.Header.Del("Content-Encoding")
			r.Header.Del("Content-Length")

			r.Body = io.NopCloser(bytes.NewReader(data))

			return nil
		},
	}

	http.ListenAndServe("0.0.0.0:80", proxy)
}

func replaceDomains(response []byte) []byte {
	re := regexp.MustCompile(`(?i)[A-Za-z\-\.]*\.?kernel\.org`)

	response = re.ReplaceAllFunc(response, func(original_raw []byte) []byte {
		kernel := strings.ToLower(string(original_raw))

		// Strip `www.`
		kernel = strings.TrimPrefix(kernel, "www.")

		yadro, exists := Kernel2Yadro(kernel)

		if !exists {
			fmt.Println("Missing:", kernel)
			return original_raw
		}

		return []byte(yadro)
	})

	response = bytes.ReplaceAll(response, []byte("%3F"), []byte("?"))
	response = bytes.ReplaceAll(response, []byte("%26"), []byte("&"))
	response = bytes.ReplaceAll(response, []byte("https"), []byte("http")) // TODO: TEMP

	return response
}

func translateWithPromtPuppies(response []byte) []byte {
	// Don't try to translate empty body (30x, etc)
	if len(response) == 0 {
		return response
	}

	req, _ := http.NewRequest("POST", "http://caddy:9000/translate", bytes.NewReader(response))

	req.Header.Add("Content-Type", "text/html")

	resp, err := http.DefaultClient.Do(req)

	if err != nil {
		fmt.Fprintln(os.Stderr, "Error in first", err)
		return []byte{0}
	}

	response, _ = io.ReadAll(resp.Body)

	resp.Body.Close()

	return response
}

func modifyResponse(response []byte) []byte {
	response = translateWithPromtPuppies(response)
	response = replaceDomains(response)
	return response
}
