# How to get the ASN.1 files

1) Use pdftotext to get the standard in a text file
```
pdftotext -layout O-RAN.WG3.E2AP-v01.01.pdf e2ap-v01.01.raw
```

2) Use the awk script to strip any non-ASN.1 text (change the standard number
   in the `extract_asn1.awk` script)
```
awk -f extract_asn1.awk e2ap-v01.01.raw > e2ap-v01.01.asn1
```

3) Strip the first characters of every line in `e2ap-v01.01.asn1`, e.g., use
`Ctrl-v` in vim and select the block, then `x`.

4) Generate the source files:
```
asn1c -gen-PER -no-gen-OER -fcompound-names -no-gen-example -findirect-choice -fno-include-deps e2ap-v01.01.asn1
```
   In my case, there were some parsing errors due to the newlines in the files
   that were introduced in the original PDF. I had to fix them manually.

# How to get the flatbuffer files:

```
flatcc -a e2ap.fbs -o .
```
creates all header files (recursively for files included in e2ap.fbs)
