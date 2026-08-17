# R002 manuscript

The public paper is available in two source forms:

- `r002-aes-sbox.md` — GitHub-renderable manuscript for immediate reading;
- `r002-aes-sbox.tex` — typeset source used to build the PDF.

Build the PDF with a standard pdfLaTeX/latexmk installation plus Ghostscript:

```bash
bash build.sh
```

The build script fixes `SOURCE_DATE_EPOCH` and PDF metadata fields to reduce nondeterministic output. CI compiles the paper from the public LaTeX source and uploads the resulting PDF as a workflow artifact. The source identities are frozen in `../artifact-manifest.json`.
