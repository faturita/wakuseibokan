##=============================================================================
##
## Makefile
## Copyright (C) 2005  Julien Guertault
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
##
##=============================================================================

NAME		=	water

DIRECTORY	=	Water

ARCHIVE		=	tar -cvzf

CC		=	gcc

ECHO		=	@echo

RM		=	rm -f

SRC		=	main.c			\
			noise.c

CFLAGS		=	-Wall			\
			-W			\
			-Wstrict-prototypes	\
			-Werror

IFLAGS		=	-I/usr/include		\
			-I/usr/include/GL	\
			-I/usr/X11R6/include	\
			-I/usr/X11R6/include/GL

LFLAGS		=	-L/usr/lib		\
			-L/usr/X11R6/lib	\
			-lX11			\
			-lGL			\
			-lGLU			\
			-lglut			\
			-ljpeg

OBJ		=	$(SRC:.c=.o)

all :		$(NAME)

$(NAME) :	$(OBJ)
		$(CC) $(OBJ) $(LFLAGS) -o $(NAME)

.c.o :
		$(CC) $(CFLAGS) $(IFLAGS) $< -c -o $@

clean :
		$(RM) $(OBJ)
		$(RM) *~ \#*\#

distclean :	clean
		$(RM) $(NAME)

separator :
		$(ECHO) "------------------------------------------"

re :		clean separator all

run :		all
		./$(NAME)

tarball :	clean separator
		$(ECHO) "Archiving..."
		cd .. && $(ARCHIVE) $(NAME).tar.gz $(DIRECTORY)
		$(ECHO) "Done !"

install :
		$(ECHO) "Not implemented yet."

love :
		$(ECHO) "Sorry, I have a headache..."
