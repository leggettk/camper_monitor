#include "WebPageBuilder.h"

void WebPageBuilder::begin(
    const String& title,
    const String& heading,
    const String& subtitle,
    uint16_t refreshSeconds)
{
    html_ = "";
    cardOpen_ = false;

    html_.reserve(5000);

    html_ += F("<!DOCTYPE html>");
    html_ += F("<html lang=\"en\">");
    html_ += F("<head>");
    html_ += F("<meta charset=\"UTF-8\">");
    html_ += F(
        "<meta name=\"viewport\" "
        "content=\"width=device-width, initial-scale=1.0\">");

    if (refreshSeconds > 0)
    {
        html_ += F("<meta http-equiv=\"refresh\" content=\"");
        html_ += String(refreshSeconds);
        html_ += F("\">");
    }
    html_ += F("<title>");
    html_ += escapeHtml(title);
    html_ += F("</title>");

    appendStyles();

    html_ += F("</head>");
    html_ += F("<body>");
    html_ += F("<main class=\"container\">");

    html_ += F("<header class=\"page-header\">");
    html_ += F("<h1>");
    html_ += escapeHtml(heading);
    html_ += F("</h1>");

    if (subtitle.length() > 0)
    {
        html_ += F("<p class=\"subtitle\">");
        html_ += escapeHtml(subtitle);
        html_ += F("</p>");
    }

    html_ += F("</header>");
}

void WebPageBuilder::beginCard(const String& title)
{
    if (cardOpen_)
    {
        endCard();
    }

    html_ += F("<section class=\"card\">");
    html_ += F("<h2>");
    html_ += escapeHtml(title);
    html_ += F("</h2>");
    html_ += F("<div class=\"status-list\">");

    cardOpen_ = true;
}

void WebPageBuilder::endCard()
{
    if (!cardOpen_)
    {
        return;
    }

    html_ += F("</div>");
    html_ += F("</section>");

    cardOpen_ = false;
}

void WebPageBuilder::addStatus(
    const String& label,
    const String& value,
    bool healthy)
{
    if (!cardOpen_)
    {
        return;
    }

    html_ += F("<div class=\"status-row\">");

    html_ += F("<span class=\"status-label\">");
    html_ += escapeHtml(label);
    html_ += F("</span>");

    html_ += F("<span class=\"status-value\">");

    html_ += healthy
        ? F("<span class=\"indicator indicator-good\"></span>")
        : F("<span class=\"indicator indicator-bad\"></span>");

    html_ += escapeHtml(value);
    html_ += F("</span>");

    html_ += F("</div>");
}

void WebPageBuilder::addParagraph(const String& text)
{
    html_ += F("<p>");
    html_ += escapeHtml(text);
    html_ += F("</p>");
}

String WebPageBuilder::build()
{
    if (cardOpen_)
    {
        endCard();
    }

    html_ += F(
        "<footer>"
        "Camper Sentinel"
        "</footer>");

    html_ += F("</main>");
    html_ += F("</body>");
    html_ += F("</html>");

    return html_;
}

void WebPageBuilder::appendStyles()
{
    html_ += F(R"rawliteral(
<style>
    :root {
        color-scheme: dark;
        font-family:
            -apple-system,
            BlinkMacSystemFont,
            "Segoe UI",
            Roboto,
            Arial,
            sans-serif;
    }

    * {
        box-sizing: border-box;
    }

    body {
        margin: 0;
        min-height: 100vh;
        background: #101419;
        color: #edf2f7;
    }

    .container {
        width: min(100%, 760px);
        margin: 0 auto;
        padding: 24px 16px 40px;
    }

    .page-header {
        margin-bottom: 24px;
    }

    h1 {
        margin: 0;
        font-size: 2rem;
        line-height: 1.2;
    }

    .subtitle {
        margin: 8px 0 0;
        color: #9ca8b5;
        font-size: 1rem;
    }

    .card {
        margin-bottom: 16px;
        padding: 18px;
        background: #1a2027;
        border: 1px solid #2a333d;
        border-radius: 14px;
        box-shadow: 0 8px 24px rgba(0, 0, 0, 0.18);
    }

    .card h2 {
        margin: 0 0 12px;
        font-size: 0.85rem;
        letter-spacing: 0.08em;
        text-transform: uppercase;
        color: #9ca8b5;
    }

    .status-list {
        display: flex;
        flex-direction: column;
    }

    .status-row {
        display: flex;
        justify-content: space-between;
        align-items: center;
        gap: 16px;
        padding: 12px 0;
        border-bottom: 1px solid #2a333d;
    }

    .status-row:last-child {
        border-bottom: none;
        padding-bottom: 0;
    }

    .status-row:first-child {
        padding-top: 0;
    }

    .status-label {
        color: #c8d0d9;
    }

    .status-value {
        display: flex;
        align-items: center;
        justify-content: flex-end;
        gap: 8px;
        text-align: right;
        font-weight: 600;
    }

    .indicator {
        width: 10px;
        height: 10px;
        flex: 0 0 10px;
        border-radius: 50%;
    }

    .indicator-good {
        background: #33d17a;
        box-shadow: 0 0 8px rgba(51, 209, 122, 0.65);
    }

    .indicator-bad {
        background: #ff5c5c;
        box-shadow: 0 0 8px rgba(255, 92, 92, 0.65);
    }

    footer {
        margin-top: 28px;
        color: #6f7b87;
        text-align: center;
        font-size: 0.8rem;
    }

    @media (max-width: 480px) {
        h1 {
            font-size: 1.7rem;
        }

        .card {
            padding: 16px;
        }

        .status-row {
            align-items: flex-start;
        }
    }
</style>
)rawliteral");
}

String WebPageBuilder::escapeHtml(const String& value) const
{
    String escaped;
    escaped.reserve(value.length() + 16);

    for (size_t index = 0; index < value.length(); ++index)
    {
        const char character = value[index];

        switch (character)
        {
            case '&':
                escaped += F("&amp;");
                break;

            case '<':
                escaped += F("&lt;");
                break;

            case '>':
                escaped += F("&gt;");
                break;

            case '"':
                escaped += F("&quot;");
                break;

            case '\'':
                escaped += F("&#39;");
                break;

            default:
                escaped += character;
                break;
        }
    }

    return escaped;
}