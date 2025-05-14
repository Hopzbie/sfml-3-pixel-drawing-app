// library
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

// function - align text
void AlignText(sf::Text& text, float x, float y)
{
    // bounding box
    sf::FloatRect bounds = text.getLocalBounds();

    // set origin
    text.setOrigin({
        x*bounds.size.x + bounds.position.x,
        y*text.getCharacterSize()
    });
}

// class - HSV color
class HSV
{
    private:

    // values
    uint16_t h;
    uint8_t  s;
    uint8_t  v;

    public:

    // constructor
    HSV(uint16_t h, uint8_t s, uint8_t v) : h(h), s(s), v(v) {}
    
    // functions - get values
    uint16_t GetH() {return h;}
    uint8_t  GetS() {return s;}
    uint8_t  GetV() {return v;}

    // functions - decrease values
    void DecreaseH() {if (h > 0) h -= 1;}
    void DecreaseS() {if (s > 0) s -= 1;}
    void DecreaseV() {if (v > 0) v -= 1;}

    // functions - increase values
    void IncreaseH() {if (h < 360) h += 1;}
    void IncreaseS() {if (s < 100) s += 1;}
    void IncreaseV() {if (v < 100) v += 1;}

    // function - convert to color
    sf::Color Color()
    {
        // constrain & normalize
        float H = (h % 360) / 60.f;
        float S = (s % 101) / 100.f;
        float V = (v % 101) / 100.f;

        // index & difference of hue
        uint8_t i = std::floor(H);
        float   D = H - i;

        // min & average value
        float M = V * (1 - S);
        float X;
        if (i % 2)  X = V + (M - V)*D;
        else        X = M + (V - M)*D;

        // scale to 0-255
        uint8_t v = std::round(V * 255);
        uint8_t m = std::round(M * 255);
        uint8_t x = std::round(X * 255);

        // color
        switch (i)
        {
            case 0:  return sf::Color(v, x, m);  break;
            case 1:  return sf::Color(x, v, m);  break;
            case 2:  return sf::Color(m, v, x);  break;
            case 3:  return sf::Color(m, x, v);  break;
            case 4:  return sf::Color(x, m, v);  break;
            case 5:  return sf::Color(v, m, x);  break;
            default: return sf::Color(0, 0, 0);
        }
    }
};

// class - color selector
class ColorSelector
{
    private:

    // shapes
    float size;
    sf::RectangleShape preview;
    sf::RectangleShape bar;

    public:

    // position
    float x = 0.f;
    float y = 0.f;

    // HSV color
    HSV hsv;

    // constructor
    ColorSelector(float size) :
        size(size),
        preview({size, size}),
        bar({size, size*0.1f}),
        hsv(200, 100, 100)
    {
        // preview
        preview.setOrigin({size/2, 0.f});

        // bar
        bar.setOrigin({0.f, size*0.05f});
    }

    // function - keyboard input
    void Keyboard()
    {
        using namespace sf::Keyboard;

        // release shift
        if (not isKeyPressed(Scan::LShift))
        {
            if (isKeyPressed(Scan::Q)) hsv.DecreaseH();
            if (isKeyPressed(Scan::E)) hsv.IncreaseH();
            if (isKeyPressed(Scan::A)) hsv.DecreaseS();
            if (isKeyPressed(Scan::D)) hsv.IncreaseS();
            if (isKeyPressed(Scan::Z)) hsv.DecreaseV();
            if (isKeyPressed(Scan::C)) hsv.IncreaseV();
        }
    }

    // function - keyboard input (event)
    void Keyboard_Event(sf::Event::KeyPressed const* pressed)
    {
        using namespace sf::Keyboard;

        // hold shift
        if (pressed->shift)
        {
            switch (pressed->scancode)
            {
                case Scan::Q:  hsv.DecreaseH();  break;
                case Scan::E:  hsv.IncreaseH();  break;
                case Scan::A:  hsv.DecreaseS();  break;
                case Scan::D:  hsv.IncreaseS();  break;
                case Scan::Z:  hsv.DecreaseV();  break;
                case Scan::C:  hsv.IncreaseV();  break;
            }
        }
    }

    // function - draw on window
    void Draw(sf::RenderWindow& window, sf::Text& text)
    {
        // set text size
        text.setCharacterSize(size*0.15f);
        
        // local position
        float x = this->x;
        float y = this->y;

        // preview
        preview.setPosition({x, y});
        preview.setFillColor(hsv.Color());
        window.draw(preview);

        // bars
        x -= size/2;
        y += size + size*0.2f;
        for (uint8_t i = 0;  i < 3;  i++)
        {
            // parameters
            char     letter;
            uint16_t value;
            float    scale;
            switch (i)
            {
                case 0: letter = 'H'; value = hsv.GetH(); scale = value/360.f; break;
                case 1: letter = 'S'; value = hsv.GetS(); scale = value/100.f; break;
                case 2: letter = 'V'; value = hsv.GetV(); scale = value/100.f; break;
            }

            // bar
            bar.setPosition({x, y});
            bar.setScale({1.f, 1.f});
            bar.setFillColor(HSV(0, 0, 50).Color());
            window.draw(bar);
            bar.setScale({scale, 1.f});
            bar.setFillColor(HSV(0, 0, 100).Color());
            window.draw(bar);

            // letter
            text.setString(letter);
            text.setPosition({x - size*0.1f, y});
            AlignText(text, 0.5f, 0.65f);
            window.draw(text);

            // value
            text.setString(std::to_string(value));
            text.setPosition({x + size + size*0.05f, y});
            AlignText(text, 0.f, 0.65f);
            window.draw(text);

            // next
            y += size*0.25f;
        }
    }
};

// class - canvas
class Canvas
{
    private:

    // size
    uint8_t w;
    uint8_t h;
    uint8_t w_new;
    uint8_t h_new;

    // scale
    float scale;

    // storage
    std::vector<uint8_t> pixels;

    // texture & sprite
    sf::Texture texture;
    sf::Sprite  sprite;

    // background
    sf::RectangleShape bg;

    public:

    // constructor
    Canvas(uint8_t w, uint8_t h, float scale = 50.f) :
        w(w),
        h(h),
        w_new(w),
        h_new(h),
        scale(scale),
        pixels(w * h * 4, 0),
        texture({w, h}),
        sprite(texture)
    {}

    // function - get width & height
    uint8_t GetW() { return w; }
    uint8_t GetH() { return h; }

    // function - save to PNG file
    void SavePNG()
    {
        bool success = texture.copyToImage().saveToFile("Draw.png");
    }

    // function - clear pixels
    void Clear()
    {
        for (auto& element : pixels)
            element = 0;
    }

    // function - update texture
    void UpdateTexture()
    {
        texture.update(pixels.data());
    }

    // function - update size
    void UpdateSize()
    {
        // copy pixels to array
        uint8_t array[h][w][4];
        for (uint8_t y = 0;  y < h;  y++)
        {
            for (uint8_t x = 0;  x < w;  x++)
            {
                // index
                uint32_t i = (x + y*w) * 4;

                // copy
                array[y][x][0] = pixels[i];
                array[y][x][1] = pixels[i+1];
                array[y][x][2] = pixels[i+2];
                array[y][x][3] = pixels[i+3];
            }
        }

        // resize pixels
        pixels.resize(w_new * h_new * 4);

        // clear pixels
        Clear();

        // paste array to pixels
        for (uint8_t y = 0;  y < h;  y++)
        {
            for (uint8_t x = 0;  x < w;  x++)
            {
                // index
                uint32_t i_new = (x + y*w_new) * 4;

                // paste
                pixels[i_new]   = array[y][x][0];
                pixels[i_new+1] = array[y][x][1];
                pixels[i_new+2] = array[y][x][2];
                pixels[i_new+3] = array[y][x][3];
            }
        }

        // update size
        w = w_new;
        h = h_new;

        // texture
        bool success = texture.resize({w, h});
        UpdateTexture();

        // sprite
        sprite.setTextureRect(sf::IntRect({0, 0}, {w, h}));
    }

    // function - increase/decrease size
    void DecreaseW() { if (w_new >   1) {w_new -= 1;  UpdateSize();} }
    void DecreaseH() { if (h_new >   1) {h_new -= 1;  UpdateSize();} }
    void IncreaseW() { if (w_new < 255) {w_new += 1;  UpdateSize();} }
    void IncreaseH() { if (h_new < 255) {h_new += 1;  UpdateSize();} }

    // function - set pixel
    void SetPixel(uint8_t x, uint8_t y, sf::Color const& color)
    {
        // check out-of-bounds
        if (x >= w or y >= h)
            return;

        // index
        uint32_t i = (x + y*w) * 4;
        
        // check change
        if (pixels[i]   == color.r and
            pixels[i+1] == color.g and
            pixels[i+2] == color.b and
            pixels[i+3] == color.a)
            return;

        // set
        pixels[i]   = color.r;
        pixels[i+1] = color.g;
        pixels[i+2] = color.b;
        pixels[i+3] = color.a;

        // update texture
        UpdateTexture();
    }

    // function - mouse input
    void Mouse(sf::RenderWindow& window, sf::Color const& color)
    {
        using namespace sf::Mouse;

        // check state
        sf::Color c;
        if      (isButtonPressed(Button::Left ))  c = color;
        else if (isButtonPressed(Button::Right))  c = sf::Color(0, 0, 0, 0);
        else    return;

        // mouse position relative to window
        sf::Vector2i position = getPosition(window);

        // convert to pixel position
        uint8_t x = position.x / scale;
        uint8_t y = position.y / scale;

        // set pixel
        SetPixel(x, y, c);
    }

    // function - keyboard input (event)
    void Keyboard_Event(sf::Event::KeyPressed const* pressed)
    {
        using namespace sf::Keyboard;

        switch (pressed->scancode)
        {
            // clear canvas with ESC
            case Scan::Escape:  Clear();  UpdateTexture();  break;

            // resize canvas with arrows
            case Scan::Left :  DecreaseW();  break;
            case Scan::Right:  IncreaseW();  break;
            case Scan::Up   :  DecreaseH();  break;
            case Scan::Down :  IncreaseH();  break;

            // save
            case Scan::S:  if (pressed->control)  SavePNG();  break;
        }
    }

    // function - zoom with mouse wheel (event)
    void Zoom_Event(sf::Event::MouseWheelScrolled const* scrolled)
    {
        float zoom = 1.1f;

        if (scrolled->delta > 0)  scale *= zoom*scrolled->delta;
        else                      scale /= zoom*abs(scrolled->delta);
    }

    // function - zoom to fit width or heigth
    void ZoomFit(float width, float height)
    {
        if (w > h)  scale = width  / w;
        else        scale = height / h;
    }

    // function - draw on window
    void Draw(sf::RenderWindow& window)
    {
        // background
        bg.setSize({w*scale, h*scale});
        bg.setFillColor(HSV(0, 0, 50).Color());
        window.draw(bg);

        // sprite
        sprite.setScale({scale, scale});
        window.draw(sprite);
    }
};

// main program
int main()
{
    // spacing
    float draw_w = 600.f;
    float draw_h = 600.f;
    float pane_w = 200.f;

    // create canvas
    Canvas canvas(5, 5);
    canvas.ZoomFit(draw_w, draw_h);

    // create pane
    sf::RectangleShape pane;
    pane.setFillColor(sf::Color(32, 32, 32));

    // create color selector
    ColorSelector selector(100.f);

    // load font & create text
    sf::Font font("C:/Windows/Fonts/arial.ttf");
    sf::Text text(font);

    // create window
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(draw_w + pane_w, draw_h)), "Title");

    // set target frame rate
    window.setFramerateLimit(60);

    // window loop
    while (window.isOpen())
    {
        // handle events
        while (auto event = window.pollEvent())
        {
            // on close button release
            if (event->is<sf::Event::Closed>())
            {
                // close window
                window.close();
            }
            
            // when window is resized
            else if (auto resized = event->getIf<sf::Event::Resized>())
            {
                // update view
                window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(resized->size))));
            
                // update size of drawing area
                draw_w = resized->size.x - pane_w;
                draw_h = resized->size.y;
            }
        
            // on mouse press
            else if (auto pressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                // zoom canvas to fit drawing area
                if (pressed->button == sf::Mouse::Button::Middle)
                    canvas.ZoomFit(draw_w, draw_h);
            }

            // on keyboard press
            else if (auto pressed = event->getIf<sf::Event::KeyPressed>())
            {
                // color selector
                selector.Keyboard_Event(pressed);

                // canvas
                canvas.Keyboard_Event(pressed);
            }

            // when mouse wheel is scrolled
            else if (auto scrolled = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                // zoom canvas
                canvas.Zoom_Event(scrolled);
            }
        }

        // check mouse input for canvas
        canvas.Mouse(window, selector.hsv.Color());

        // check keyboard input for color selector
        selector.Keyboard();

        // fill background color
        window.clear(sf::Color(64, 64, 64));
        
        // draw canvas
        canvas.Draw(window);

        // draw pane
        pane.setSize({pane_w, draw_h});
        pane.setPosition({draw_w, 0.f});
        window.draw(pane);

        // draw color selector
        selector.x = draw_w + pane_w/2;
        selector.y = 20.f;
        selector.Draw(window, text);

        // draw canvas size
        float x = draw_w + pane_w/2;
        float y = draw_h - 10.f;
        text.setCharacterSize(20);
        text.setString('x');
        text.setPosition({x, y});
        AlignText(text, 0.5f, 1.f);
        window.draw(text);
        // width
        text.setString(std::to_string(canvas.GetW()));
        text.setPosition({x - 20.f, y});
        AlignText(text, 1.f, 1.f);
        window.draw(text);
        // width
        text.setString(std::to_string(canvas.GetH()));
        text.setPosition({x + 20.f, y});
        AlignText(text, 0.f, 1.f);
        window.draw(text);

        // update display
        window.display();
    }

    // program end succesfully
    return 0;
}