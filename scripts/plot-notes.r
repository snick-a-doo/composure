## Copyright © 2020-2021 Sam Varner
##
## This file is part of Composure.
##
## Composure is free software: you can redistribute it and/or modify it under the terms of
## the GNU General Public License as published by the Free Software Foundation, either
## version 3 of the License, or (at your option) any later version.
##
## Composure is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
## without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
## PURPOSE.  See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along with Composure.
## If not, see <http://www.gnu.org/licenses/>.

## Plot the notes in a .notes file.  Bars show note duration.  Color shows generation.
plot.notes <- function(file) {
    notes <- read.table(file)
    ## Plot points for note-on times and connect with lines.
    plot(notes$V1/60, notes$V3, type='o', xlab="Time (min)", ylab="Pitch")
    ## Plot horizontal bars colored by pitch for the duration of each note.
    apply(notes, 1, function(x) lines(c(x[1], x[2])/60, c(x[3], x[3]), lwd=5, col=x[4]+1))
}
