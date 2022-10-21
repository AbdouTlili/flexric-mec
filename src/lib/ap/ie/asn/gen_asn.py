import os
import sys



def gen_asn(pdf_file):
    cmd_totxt = "pdftotext -layout " + pdf_file + " /tmp/tmp.txt"
    # E42 diff file with a w at the end so that ed can write
    cmd_toE42 = "patch e2ap-v01.01.asn1 e42.patch"
    cmd_toasn = 'asn1c -gen-PER -no-gen-OER -fcompound-names -no-gen-example -findirect-choice -fno-include-deps e2ap-v01.01.asn1'


    os.system(cmd_totxt)

    fd_o = open('e2ap-v01.01.asn1', 'w' )
    fd_i = open('/tmp/tmp.txt', 'r')

    start = False
    start_comments = False

    prev_line = ''

    str_except = 'XnAP, etc'

    for line in fd_i:

        if start == False:
            idx = line.find('ASN1START')
            if idx > -1:
                start = True
        else:
            idx = line.find('Copyright') 
            if idx > -1:
                continue

            idx = line.find('O-RAN.WG3.E2AP-v01.01') 
            if idx > -1:
                continue

            idx = line.find('______________________') 
            if idx > -1:
                continue

            idx = line.find('ASN1STOP') 
            if idx > -1:
                start = False
                continue

            line = line[5:]

            if prev_line != '':
                line = prev_line + line 

            if len(line) < 2:
                continue

            idx = line.find('********************************************')
            if(idx > -1):
                if start_comments == True:
                    start_comments = False
                else:
                    start_comments = True
            
            if start_comments == True:
                continue

            idx = line.find(str_except)
            if idx > -1:
                continue

            if line[len(line) -2] == '-':
                prev_line = line[:-1] 
            else:
                prev_line = ''
                fd_o.writelines(line)
            
    # Closing files
    fd_i.close()
    fd_o.close()

    # E42 script
    os.system(cmd_toE42)

    # ASN.1 Compiler
    os.system(cmd_toasn)

def main():
    pdf_file = sys.argv[1]

    idx = pdf_file.find('.docx')  
    if(idx != -1):
        print('.docx file passed. Converting to pdf...' )
        cmd_to_pdf = 'lowriter --convert-to pdf ' + pdf_file + ' --outdir /tmp/fr_tmp.pdf' 
        os.system(cmd_to_pdf)
        pdf_file = '/tmp/fr_tmp.pdf'


    idx = pdf_file.find('.pdf')
    if(idx == -1):
        print('Pdf file passed or converted from .docx = ' + pdf_file)
        print('Invalid pdf file. Pass a .pdf file e.g., python3 gen_asn.py O-RAN.WG3.E2AP-v01.01.pdf')
        return
    
    gen_asn(pdf_file)



if __name__=="__main__":
    main()


