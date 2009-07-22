#! /bin/sh

#
#   simple tests for playwav
#
#   Copyright (c) Paul Downey (@psd), Andrew Back (@9600) 2009
#   
# 
#    This program is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
# 
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU General Public License
#     along with this program.  If not, see <http://www.gnu.org/licenses/>
# 
# 

[ ! -d out ]&&mkdir out
function expect()
{
	echo "$1: \c"
	eval $2 > out/$1.stdout 2> out/$1.stderr
	if cmp tests/$1.stderr out/$1.stderr > out/$1.cmp
	then
		echo "[PASS]"
	else
		diff tests/$1.stderr out/$1.stderr >> out/$1.cmp
		echo "[FAIL]"
	fi
}

expect usage 'playwav -h' 
