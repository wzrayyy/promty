package main

import (
    "fmt"
    "io"
    "bytes"
    "regexp"
    "strings"
    "compress/gzip"
    "net/url"
    "net/http"
    "net/http/httputil"
    "golang.org/x/text/encoding/charmap"
)

func main() {
    proxy := &httputil.ReverseProxy{
        Rewrite: func(r *httputil.ProxyRequest) {
            host, _ := Yadro2Kernel(r.In.Host, true)
            url, err := url.Parse("https://" + host)
            
            if err != nil {
                // TODO...
            }

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

            // Modify the response.
            data = modifyResponse(data)

            // Remove headers that mess with body encoding and set the body.
            r.Header.Del("Content-Encoding")
            r.Header.Del("Content-Length")
            // r.Header.Set("Content-Type", "text/html; charset=windows-1251")

            r.Body = io.NopCloser(bytes.NewReader(data))
            
            return nil
        },
    }

    http.ListenAndServe("localhost:8000", proxy)
}

func replaceDomains(response []byte) []byte {
    re := regexp.MustCompile(`(?i)[A-Za-z\-\.]*\.?kernel\.org`)
    response = re.ReplaceAllFunc(response, func(original_raw []byte) []byte {
        kernel := strings.ToLower(string(original_raw))

        // Strip `www.`
        kernel = strings.TrimPrefix(kernel, "www.")

        yadro, exists := Kernel2Yadro(kernel)

        if !exists {
            return original_raw
        }

        return []byte(yadro)
    })

    return response
}

func translateWithPromtPuppies(response []byte) []byte {
    enc := charmap.Windows1251.NewEncoder()
    cp1521, _ := enc.Bytes(response)

    req, _ := http.NewRequest("POST", "http://localhost:2390/", bytes.NewReader(cp1521))  // TODO

    resp, err := http.DefaultClient.Do(req)
    fmt.Println(err)
    fmt.Println(resp.StatusCode)

    response, _ = io.ReadAll(resp.Body)
    dec, _ := charmap.Windows1251.NewDecoder().Bytes(response)
    // fmt.Println(len(dec))
    // fmt.Println(string(dec))

    resp.Body.Close()
    
    return dec
}

func modifyResponse(response []byte) []byte {
    response = replaceDomains(response)
    response = translateWithPromtPuppies(response)
    return response
}
