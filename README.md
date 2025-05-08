# DWARF-Golang

A small c tool to analyze custom DWARF attributes in golang binaries.

## Introduction

Go emits custom DWARF attributes (like `DW_AT_go_kind`, `DW_AT_go_elem`, etc...) to describe internal type information such as whether something is a `map`, `func`, `chan`, etc... These are not standard DWARF fields and are often marked as "Unknown AT value" by tools like `readelf`.

This project helps reverse engineers make sense of those attributes by summarizing how often each golang kind appears and in what DWARF tags.

[`dwarf.go`](https://github.com/golang/go/blob/master/src/cmd/internal/dwarf/dwarf.go#L312) gives information on how go types are represented in DWARF, knowing these attributes enabled me to do this 
tool to get stats of go types from DWARF debug information.

## Compile

```bash
gcc dwarf-go-stats.cc -o dwarf-go -Wall
```
## Usage

```bash
readelf --debug-dump=info your-go-binary > output.txt
./dwarf-go output.txt
```

Alternatively, you can use the hello go binary found in `examples/` folder or just compile and run it against `examples/output.txt` directly.

## Reference

- https://github.com/troydhanson/uthash
- https://go.dev/src/runtime/typekind.go
- https://eli.thegreenplace.net/2020/embedding-in-go-part-1-structs-in-structs

## LICENSE

This project is licensed under GNU v3 (General Public License), see [LICENSE](LICENSE) file for more information.
