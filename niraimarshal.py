from cStringIO import StringIO
import marshal
import struct
import types

# For obfuscation, we prepend 0x45 to the code so the interpreter knows it's obfuscated.
# Note 0x45 isn't a valid opcode.

def niraicall_obfuscate(code):
    print '''Error:
    niraicall_obfuscate not implemented
    
    Add it to your make file:
    
    def niraicall_obfuscate(code):
        ...
        
    niraimarshal.niraicall_obfuscate = niraicall_obfuscate
    
    Input: string
    Output: (bool, string)
    
    bool indicates whether the string has been obfuscated or not
    if True, OBFS is prepend to string
    if False, string is ignored 
    
    See sample project for help
    '''
    raise NotImplementedError('niraicall_obfuscate not implemented')

def obfuscate(x):
    obfuscated, code = niraicall_obfuscate(x)
    if obfuscated:
        return '\x45' + code
        
    else:
        return x

def dump(value, file):    
    if isinstance(value, types.CodeType):
        dump_code(value, file)

    elif type(value) in (list, tuple):
      #  print 'list or tuple'
        file.write('[' if type(value) == list else '(')
        file.write(struct.pack('<I', len(value)))
        for x in value:
            dump(x, file)
            
    elif type(value) == dict:
        print 'dict'
        file.write('{')
        for k, v in value.items():
            dump(k, file)
            dump(v, file)
        file.write('0')

    else:
        file.write(marshal.dumps(value))

def dump_code(value, file):
    file.write(struct.pack(
        '<cIIII', 'c', value.co_argcount, value.co_nlocals,
                       value.co_stacksize, value.co_flags))

    code = value.co_code
    dump(obfuscate(code), file)
    dump(value.co_consts, file)
    dump(value.co_names, file)
    dump(value.co_varnames, file)
    dump(value.co_freevars, file)
    dump(value.co_cellvars, file)
    dump(value.co_filename, file)
    dump(value.co_name, file)
    file.write(struct.pack('<I', value.co_firstlineno))
    dump(value.co_lnotab, file)

def dumps(value):
    sio = StringIO()
    dump(value, sio)
    return sio.getvalue()
