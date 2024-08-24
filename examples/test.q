use person as struct {
    i8* name,
    i8 age,
}

def +person(i8* name, i8 age) {
    self.name = name
    self.age = age
}

def i32 printf(i8*, ?)

def void person:print() = printf("{ name = %s, age = %d }", self.name, self.age)

use name as struct {
    i8* surname,
    i8* name
}

def +name() {
    self.surname = "Smith"
    self.name = "John"
}

def void name:print() = printf("%s %s", self.name, self.surname)

def i32 main(i32 argc, i8** argv) {

    def name john
    john.print()
    printf("\n")

    def person felix = person("Felix", 18)
    def person max = person("Max", 20)

    printf("felix = ")
    felix.print()
    printf("\n")
    printf("max = ")
    max.print()
    printf("\n")

    return 0
}
