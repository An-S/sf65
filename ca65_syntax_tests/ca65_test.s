.feature    labels_without_colons
label
    label2:
    .byte 123, 145, 156 ;Ein Kommentar
.macpack    cbm ; import cbm package
label3  .proc   fdfdf ;Noch ein Kommentar ;comment may follow always
        .proc hgfhgfh
        .endproc
        .endproc
        lda .lobyte(1234) ;directive my follow mnemonic

