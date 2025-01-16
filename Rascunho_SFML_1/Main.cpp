#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>

/*
Examples for a sphere following the properties below
radius = 10 (in centimeters)
m = 1 (in kg)
g = 9.81m/s� (gravity for Earth)
p = 1,225kg / m� (air density usually)
A (for Spheres) = pi * (radius in meters)� ->Result is in square meters 0,0314(m�)
A (for Rectangles) = width * height
A (for Elipses) = pi * semi-major axis * semi-minor axis
A (for Triangles) = (1 / 2) * base * height
Cd = 0.5 (drag coefficient of spheres. may vary)
terminalVelocity = sqroot( ( 2 * mass(1) * gravity(9.81)) / (( p(1,255) * A(0,0314) * Cd(0.5)
*/
class Particles
{   
public:
    static bool reset;
    static float gravity;
    static float airDensity;
    static constexpr float deltaTime = 0.0083;
    static constexpr float PI = 3.14159f;

    Particles(std::string shape, float radius, float weight, unsigned int windowWidth, unsigned int windowHeight,
              float width = 0.0f, float height = 0.0f, float elasticity = 0.0f)
    {
        this->shape = shape;
        //  ---------------------------------------------------
        this->radius = radius;
        //  ---------------------------------------------------
        this->weight = weight;
        //  ---------------------------------------------------
        this->width = width;
        //  ---------------------------------------------------
        this->height = height;
        //  ---------------------------------------------------
        this->elasticity = elasticity;
        //  ---------------------------------------------------
        this->setInitialValues(windowWidth, windowHeight);
        //  ---------------------------------------------------
    }

    float getWidth() const { return this->width; }
    float getHeight() const { return this->height; }
    float getAxisX() const { return this->axisX; }
    float getAxisY() const { return this->axisY; }
    float getRadius() const { return this->radius; }
    float getVelocity() const { return this->velocity; }

    void setWidth(float width) { this->width = width; }
    void setHeight(float height) { this->height = height; }

    static std::vector<Particles> resetValues(std::vector<Particles> particlesArray, unsigned int windowWidth, unsigned int windowHeight)
    {
        for (Particles& particle : particlesArray) 
        {
            particle.velocity = 0.0f;
            particle.setInitialValues(windowWidth, windowHeight);
        }

        return particlesArray;
    }

    static std::vector<Particles> update(std::vector<Particles> particlesArray, float windowWidth, float windowHeight)
    {
        for (Particles &particle : particlesArray) 
        {   
            if (!particle.isGoingUp && !particle.isStall)
            {
                particle.axisY += particle.velocity; // Updates current particle's axis Y
                particle.velocity += Particles::gravity * Particles::deltaTime; // Updates current particle's velocity
               
                if (particle.velocity >= particle.terminalVelocity)
                    particle.velocity = particle.terminalVelocity; // Sets particle's velocity to limit 

                if (particle.axisY >= windowHeight - particle.height)
                {
                    particle.axisY = windowHeight - particle.height; // Puts particle directly onto the floor
                    if (particle.velocity <= particle.terminalVelocity * 0.04f)
                    {
                        particle.isStall = true;
                        particle.velocity = 0.0f;
                        continue;
                    }                    

                    particle.isGoingUp = true;
                    particle.velocity *= particle.elasticity;
                }
            }
            else if (particle.isGoingUp && !particle.isStall)
            {
                particle.axisY -= particle.velocity;
                particle.velocity -= Particles::gravity * Particles::deltaTime;

                if (particle.velocity <= 0.0f) 
                {
                    particle.velocity = 0.0f;
                    particle.isGoingUp = false;
                }
            }
        }

        return particlesArray;
    }

private:
    std::string shape;
    bool isStall = false;
    bool isGoingUp = false;
    float radius = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float weight = 0.0f;
    float axisX = 0.0f;
    float axisY = 0.0f;
    float elasticity = 0.0f;    // Coefficient of restitution
    float velocity = 0.0f;      // Current falling velocity
    float terminalVelocity = 0.0f;            // Terminal velocity
    float A = 0.0f;             // Cross-sectional area
    float Cd = 0.0f;            // Drag coefficient

    std::optional<float> semiMajorAxis;
    std::optional<float> semiMinorAxis;

    void setInitialValues(unsigned int windowWidth, unsigned int windowHeight)
    {
        if (this->shape == "CircleShape")
        {
            this->axisX = (windowWidth / 2) - this->radius;
            this->axisY = (windowHeight / 2) - this->radius;
            this->width = this->radius * 2;
            this->height = this->radius * 2;
            this->A = Particles::PI * std::pow(radius / 100.0f, 2); // Divides radius by 100 to get radius to centimeters
            this->Cd = 0.5f;
            this->terminalVelocity = sqrt((2 * weight * Particles::gravity) / (Particles::airDensity * this->A * this->Cd));
        }
        else if (this->shape == "RectangleShape")
        {
            this->axisX = (windowWidth / 2) - this->width / 2;
            this->axisY = (windowHeight / 2) - this->height / 2;
            this->A = (this->width * this->height);
            this->Cd = 0.5f;
            this->terminalVelocity = sqrt((2 * weight * Particles::gravity) / (Particles::airDensity * this->A * this->Cd));
        }
        else if (this->shape == "TriangleShape")
        {
            this->axisX = (windowWidth / 2) - this->width / 2;
            this->axisY = (windowHeight / 2) - this->height / 2;
            this->A = (1 / 2) * this->width * this->height;
            this->Cd = 0.5f;
            this->terminalVelocity = sqrt((2 * weight * Particles::gravity) / (Particles::airDensity * this->A * this->Cd));
        }
        this->isStall = false;
        this->isGoingUp = false;
    }
};

bool Particles::reset = false;
float Particles::gravity = 0.981f;
float Particles::airDensity = 1.225f;

void renderingThread(sf::RenderWindow* window, unsigned int windowWidth, unsigned int windowHeight)
{    
    // activate the window's context
    window->setActive(true);

    std::vector<Particles> particlesArray;
    particlesArray.emplace_back("CircleShape", 15.0f, 1.7f, windowWidth, windowHeight, 0.f, 0.f, 0.6f);
    
    sf::CircleShape circleShape(particlesArray[0].getRadius());
    sf::Vector2<float> position(particlesArray[0].getAxisX(), particlesArray[0].getAxisY());
    
    circleShape.setFillColor(sf::Color::White);
    circleShape.setPosition(position);

    // the rendering loop
    while (window->isOpen())
    {
        if (Particles::reset) 
        {
            particlesArray = Particles::resetValues(particlesArray, windowWidth, windowHeight);
            Particles::reset = false;
        }

        // clear...
        window->clear();
        // draw...
        window->draw(circleShape);
        // end the current frame
        window->display();
        // update...
        particlesArray = Particles::update(particlesArray, windowWidth, windowHeight);
        position.x = particlesArray[0].getAxisX();
        position.y = particlesArray[0].getAxisY();

        circleShape.setPosition(position);
    }
}

int main()
{
    unsigned int windowWidth = 800, windowHeight = 800;

    // create the window
    sf::RenderWindow window(sf::VideoMode({ windowWidth, windowHeight }), "OpenGL", sf::Style::Default);
    sf::ContextSettings settings;
    settings.depthBits = 32;
    // window.setVerticalSyncEnabled(true);

    // deactivate its OpenGL context
    window.setActive(false);

    // launch the rendering thread
    std::thread thread(&renderingThread, &window, windowWidth, windowHeight);

    while (window.isOpen())
    {
        // aqui eu pego
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scan::R)
                {
                    std::cout << "Reseting\n";
                    Particles::reset = true;
                }
            }
        }
    }

    // release resources...
    thread.join();
}