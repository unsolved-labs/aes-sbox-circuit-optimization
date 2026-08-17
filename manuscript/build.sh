#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
command -v latexmk >/dev/null
command -v gs >/dev/null
export SOURCE_DATE_EPOCH=0
export TZ=UTC
latexmk -C >/dev/null 2>&1 || true
latexmk -pdf -interaction=nonstopmode -halt-on-error r002-aes-sbox.tex
gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.5 -dPDFSETTINGS=/prepress \
  -dNOPAUSE -dQUIET -dBATCH \
  -sOutputFile=r002-aes-sbox.optimized.pdf r002-aes-sbox.pdf
mv r002-aes-sbox.optimized.pdf r002-aes-sbox.pdf
