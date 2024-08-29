def void* malloc(i64)
def void* realloc(void*, i64)
def void free(void*)
def i64 strlen(i8*)
def i64 strcpy(i8*, i8*)

use string as struct {
    i64 length,
    i64 reserved,
    i8* buffer
}

# create empty string #
def +string() {
    self.length = 0
    self.reserved = 8
    self.buffer = malloc(self.reserved)

    self.buffer[0] = 0
}

# create string with one char #
def +string(i8 chr) {
    self.length = 1
    self.reserved = 8
    self.buffer = malloc(self.reserved)

    self.buffer[0] = chr
    self.buffer[1] = 0
}

# create string from c string #
def +string(i8* str) {
    self.length = strlen(str)
    self.reserved = self.length + 1
    self.buffer = malloc(self.reserved)

    strcpy(self.buffer, str)
}

# copy string #
def +string(string str) {
    self.length = str.length
    self.reserved = str.reserved
    self.buffer = malloc(self.reserved)

    strcpy(self.buffer, str.buffer)
}

# destroy string #
def -string() {
    free(self.buffer)
}

# clear string #
def void string:clear() {
    self.length = 0
    self.buffer[0] = 0
}

# append char to string #
def void string:append(i8 chr) {
    if self.length >= self.reserved {
        self.reserved = self.length * 2 + 1
        self.buffer = realloc(self.buffer, self.reserved)
    }
    self.buffer[self.length++] = chr
    self.buffer[self.length] = 0
}

# append c string to string #
def void string:append(i8* str) {
    self.length += strlen(str)
    if self.length >= self.reserved {
        self.reserved = self.length * 2 + 1
        self.buffer = realloc(self.buffer, self.reserved)
    }
    strcat(self.buffer, str)
}

# append string to other string #
def void string:append(string str) {
    self.length += str.length
    if self.length >= self.reserved {
        self.reserved = self.length * 2 + 1
        self.buffer = realloc(self.buffer, self.reserved)
    }
    strcat(self.buffer, str.buffer)
}

# concat 2 strings #
def string concat(string a, string b) { # args destructor does not get called as they are owned by another scope #
    def string ab
    ab.append(a)
    ab.append(b)
    return ab # destructor is not called because it leaves the scope through return #
}

def i32 main(i32 argc, i8** argv) {
    if argc < 3 return 1

    def string a = argv[1]
    def string b = argv[2]

    def string ab = concat(a, b)

    puts(ab.buffer)
    return 0
}
