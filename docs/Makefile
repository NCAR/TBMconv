
TARGET = $(patsubst %.tex,%.pdf,$(wildcard *.tex))

$(TARGET): %.pdf: %.tex
	pdflatex $<
