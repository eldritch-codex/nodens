import nodens;
import Example.CircularWave3DLayer;
import std;

// -----------------------------------------------------------------------------
// Step 2: Define the Application class, the root object in charge of the
// application, managing the window, layers, and the main event loop.
// -----------------------------------------------------------------------------

class CircularWave3DApp : public Nodens::Application
{
public:
    static inline const Nodens::FApplicationSpecification appSpecifications = {
        .Name = "[NodensApp Example] CircularWave3D",
        .WindowWidth = 800,
        .WindowHeight = 600,
        .EnableGUI = true,
    };

    CircularWave3DApp() : Application(appSpecifications)
    {
        PushLayer(new CircularWave3DLayer());
    }

    ~CircularWave3DApp() = default;
};

int main()
{
    Nodens::InitializeLoggers();

    auto app = CircularWave3DApp();
    app.Run();

    return 0;
}
