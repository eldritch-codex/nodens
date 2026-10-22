import nodens;
import Example.PlanetaryScanLayer;

// -----------------------------------------------------------------------------
// Step 3: Define the Application class, the root object in charge of the
// application, managing the window, layers, and the main event loop.
// -----------------------------------------------------------------------------

class PlanetaryScanApp : public Nodens::Application
{
public:
    static inline const Nodens::FApplicationSpecification appSpecifications = {
        .Name = "[NodensApp Example] PlanetaryScans",
        .WindowWidth = 800,
        .WindowHeight = 600,
        .EnableGUI = true,
    };

    PlanetaryScanApp() : Application(appSpecifications)
    {
        PushLayer(new PlanetaryScanLayer());
    }

    ~PlanetaryScanApp() = default;
};

// -----------------------------------------------------------------------------
// Step 4: Application entry point.
// -----------------------------------------------------------------------------

int main()
{
    Nodens::InitializeLoggers();

    auto app = PlanetaryScanApp();
    app.Run();

    return 0;
}
