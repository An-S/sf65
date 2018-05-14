# sf65 docs

## 1. Linefeeds and empty lines

### 1.1 Linefeeds and empty lines in input file

### 1.2 Generated linefeeds

#### 1.1.1 Extra linefeed before statement
This is used to produce an empty line before emitting a statement. F.e.: .PROC
is separated from code above by one empty line, if specified. This also holds for
other scoping directives like .if, .scope, <em>.repeat etc</em>...

The source formatter emits a linebreak directly when such a statement is found and if the corresponding command line option -e1 is specified (default). The linefeed is emitted before the statement is emitted into the output file.
If there was an empty line before the scope opening directive, the additional linefeed is suppressed.

#### 1.1.2 Extra linefeed directly after statement
If a line contains a label and several other statements, command line option can be given to output label into it's own line, output a linebreak directly after the label and then emit the following statements into the next line.
This is called INSTANT_ADDITIONAL_LINEFEED.

#### 1.1.3 Extra linefeed at end of line (to produce empty line after)
This is used to produce an empty line after a line containing and directive which closes a local scope. F.e.: .ENDPROC, .ENDIF, .ENDREP ...
This statements is separated from the code blow by one empty line, if specified via command line option. 
If such a statement is found, the flag ADDITIONAL_LINEFEED is set to request an empty line after.
The flag is not cleared as long statements are beeing parsed within the current line.
After the parsing of the current line has been finished, the parser continues with reading the next line. If this line is not empty, the mentioned flag is tested and then a linefeed is emitted. If this line is empty, anyway, an additional linefeed is suppressed.



