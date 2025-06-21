class Dog:
    species = "Canis familiaris" 

    def __init__(self, name):
        self.name = name

    @classmethod
    def get_species(cls):
        return cls.species

print(Dog.get_species())  