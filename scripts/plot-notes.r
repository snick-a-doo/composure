plot.notes <- function(file) {
    notes <- read.table(file)
    ## Plot points for note-on times and connect with lines.
    plot(notes$V1, notes$V3, type='o')
    ## Plot horizontal bars colored by pitch for the duration of each note.
    apply(notes, 1, function(x) lines(c(x[1], x[2]), c(x[3], x[3]), lwd=5, col=x[4]+1))
}
