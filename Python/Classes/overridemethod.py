class Animal:
    def speak(self):
        print("Animal speaks")

class Bird(Animal):
    def speak(self):
        print("Bird chirps")

bird = Bird()
bird.speak()