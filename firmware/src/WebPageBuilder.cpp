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
    StatusLevel level)
{
    if (!cardOpen_)
    {
        return;
    }

    const String levelClass = statusClass(level);

    html_ += F("<div class=\"status-row\">");

    html_ += F("<span class=\"status-label\">");
    html_ += escapeHtml(label);
    html_ += F("</span>");

    html_ += F("<span class=\"status-value\">");

    html_ += F("<span class=\"indicator ");
    html_ += levelClass;
    html_ += F("\"></span>");

    html_ += escapeHtml(value);
    html_ += F("</span>");

    html_ += F("</div>");
}


String WebPageBuilder::build()
{
    if (gridOpen_)
    {
        endGrid();
    }

    if (cardOpen_)
    {
        endCard();
    }

    if (!footerAdded_)
    {
        html_ += F(
            "<footer>"
            "<strong>Camper Sentinel</strong>"
            "<span>Know your camper. Anywhere.</span>"
            "</footer>");
    }

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

        --background: #0b1015;
        --surface: #151c23;
        --surface-raised: #1b242d;
        --border: #29343f;
        --text: #f1f5f9;
        --muted: #94a3b2;

        --normal: #31d17c;
        --warning: #f5b942;
        --critical: #ff6262;
        --neutral: #7e8b98;
        --accent: #4c98ff;
    }

    * {
        box-sizing: border-box;
    }

    body {
        margin: 0;
        min-height: 100vh;
        background:
            radial-gradient(
                circle at top right,
                rgba(76, 152, 255, 0.12),
                transparent 32rem),
            var(--background);
        color: var(--text);
    }

    .container {
        width: min(100%, 960px);
        margin: 0 auto;
        padding: 28px 18px 44px;
    }

    .page-header h1 {
        margin: 0;
        font-size: clamp(2rem, 6vw, 3.1rem);
        line-height: 1.05;
        letter-spacing: -0.04em;
    }

    .subtitle {
        margin: 10px 0 0;
        color: var(--muted);
        font-size: 1rem;
    }

    .nav {
        display: flex;
        gap: 6px;
        margin: 24px 0;
        padding: 6px;
        overflow-x: auto;
        background: rgba(21, 28, 35, 0.88);
        border: 1px solid var(--border);
        border-radius: 13px;
    }

    .nav a {
        padding: 10px 14px;
        color: var(--muted);
        text-decoration: none;
        white-space: nowrap;
        border-radius: 9px;
        font-size: 0.9rem;
        font-weight: 650;
    }

    .nav a:hover,
    .nav a.active {
        background: var(--surface-raised);
        color: var(--text);
    }

    .health-banner {
        display: flex;
        align-items: center;
        gap: 16px;
        margin-bottom: 18px;
        padding: 20px;
        border: 1px solid var(--border);
        border-radius: 16px;
        background: var(--surface);
    }

    .health-icon {
        width: 18px;
        height: 18px;
        flex: 0 0 18px;
        border-radius: 50%;
    }

    .health-title {
        font-size: 1.1rem;
        font-weight: 750;
    }

    .health-message {
        margin-top: 4px;
        color: var(--muted);
        line-height: 1.4;
    }

    .health-banner.status-normal .health-icon {
        background: var(--normal);
        box-shadow: 0 0 18px rgba(49, 209, 124, 0.65);
    }

    .health-banner.status-warning .health-icon {
        background: var(--warning);
        box-shadow: 0 0 18px rgba(245, 185, 66, 0.55);
    }

    .health-banner.status-critical .health-icon {
        background: var(--critical);
        box-shadow: 0 0 18px rgba(255, 98, 98, 0.55);
    }

    .health-banner.status-neutral .health-icon {
        background: var(--neutral);
    }

    .metric-grid {
        display: grid;
        grid-template-columns:
            repeat(3, minmax(0, 1fr));
        gap: 14px;
        margin-bottom: 18px;
    }

    .metric-card {
        position: relative;
        overflow: hidden;
        min-height: 144px;
        padding: 19px;
        border: 1px solid var(--border);
        border-radius: 16px;
        background:
            linear-gradient(
                145deg,
                var(--surface-raised),
                var(--surface));
    }

    .metric-card::before {
        position: absolute;
        top: 0;
        left: 0;
        width: 100%;
        height: 3px;
        content: "";
        background: var(--neutral);
    }

    .metric-card.status-normal::before {
        background: var(--normal);
    }

    .metric-card.status-warning::before {
        background: var(--warning);
    }

    .metric-card.status-critical::before {
        background: var(--critical);
    }

    .metric-title {
        color: var(--muted);
        font-size: 0.82rem;
        font-weight: 700;
        letter-spacing: 0.08em;
        text-transform: uppercase;
    }

    .metric-value {
        margin-top: 20px;
        font-size: clamp(1.65rem, 5vw, 2.4rem);
        font-weight: 780;
        letter-spacing: -0.035em;
    }

    .metric-detail {
        margin-top: 7px;
        color: var(--muted);
        font-size: 0.88rem;
    }

    .card {
        margin-bottom: 16px;
        padding: 20px;
        background: var(--surface);
        border: 1px solid var(--border);
        border-radius: 16px;
    }

    .card h2 {
        margin: 0 0 14px;
        color: var(--muted);
        font-size: 0.82rem;
        letter-spacing: 0.1em;
        text-transform: uppercase;
    }

    .status-row {
        display: flex;
        justify-content: space-between;
        align-items: center;
        gap: 18px;
        padding: 13px 0;
        border-bottom: 1px solid var(--border);
    }

    .status-row:first-child {
        padding-top: 0;
    }

    .status-row:last-child {
        padding-bottom: 0;
        border-bottom: none;
    }

    .status-label {
        color: #cbd5df;
    }

    .status-value {
        display: flex;
        align-items: center;
        justify-content: flex-end;
        gap: 9px;
        text-align: right;
        font-weight: 650;
    }

    .status-value.plain {
        color: var(--text);
    }

    .indicator {
        width: 9px;
        height: 9px;
        flex: 0 0 9px;
        border-radius: 50%;
        background: var(--neutral);
    }

    .indicator.status-normal {
        background: var(--normal);
        box-shadow: 0 0 8px rgba(49, 209, 124, 0.6);
    }

    .indicator.status-warning {
        background: var(--warning);
    }

    .indicator.status-critical {
        background: var(--critical);
    }

    a {
        color: #86baff;
    }

    footer {
        display: flex;
        flex-wrap: wrap;
        justify-content: center;
        gap: 8px 18px;
        margin-top: 30px;
        color: #6f7d8b;
        text-align: center;
        font-size: 0.79rem;
    }

    footer strong {
        color: #93a0ad;
    }

    @media (max-width: 720px) {
        .metric-grid {
            grid-template-columns: 1fr;
        }

        .metric-card {
            min-height: 122px;
        }
    }

    @media (max-width: 480px) {
        .container {
            padding: 21px 13px 34px;
        }

        .status-row {
            align-items: flex-start;
        }

        .status-value {
            max-width: 58%;
        }

        .health-banner {
            padding: 17px;
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
void WebPageBuilder::addNavigation(
    const String& activePage)
{
    html_ += F("<nav class=\"nav\">");

    struct NavItem
    {
        const char* label;
        const char* path;
        const char* id;
    };

    const NavItem items[] =
    {
        {"Dashboard", "/", "dashboard"},
        {"Settings", "/settings", "settings"},
        {"Diagnostics", "/diagnostics", "diagnostics"},
        {"Firmware", "/ota", "firmware"}
    };

    for (const NavItem& item : items)
    {
        html_ += F("<a href=\"");
        html_ += item.path;
        html_ += F("\"");

        if (activePage == item.id)
        {
            html_ += F(" class=\"active\"");
        }

        html_ += F(">");
        html_ += item.label;
        html_ += F("</a>");
    }

    html_ += F("</nav>");
}
void WebPageBuilder::addHealthBanner(
    const String& title,
    const String& message,
    StatusLevel level)
{
    const String levelClass = statusClass(level);

    html_ += F("<section class=\"health-banner ");
    html_ += levelClass;
    html_ += F("\">");

    html_ += F("<div class=\"health-icon\"></div>");

    html_ += F("<div>");
    html_ += F("<div class=\"health-title\">");
    html_ += escapeHtml(title);
    html_ += F("</div>");

    html_ += F("<div class=\"health-message\">");
    html_ += escapeHtml(message);
    html_ += F("</div>");
    html_ += F("</div>");

    html_ += F("</section>");
}
void WebPageBuilder::beginGrid()
{
    if (gridOpen_)
    {
        return;
    }

    html_ += F("<section class=\"metric-grid\">");
    gridOpen_ = true;
}

void WebPageBuilder::endGrid()
{
    if (!gridOpen_)
    {
        return;
    }

    html_ += F("</section>");
    gridOpen_ = false;
}
void WebPageBuilder::addMetricCard(
    const String& title,
    const String& value,
    const String& detail,
    StatusLevel level)
{
    const String levelClass = statusClass(level);

    html_ += F("<article class=\"metric-card ");
    html_ += levelClass;
    html_ += F("\">");

    html_ += F("<div class=\"metric-title\">");
    html_ += escapeHtml(title);
    html_ += F("</div>");

    html_ += F("<div class=\"metric-value\">");
    html_ += escapeHtml(value);
    html_ += F("</div>");

    html_ += F("<div class=\"metric-detail\">");
    html_ += escapeHtml(detail);
    html_ += F("</div>");

    html_ += F("</article>");
}
void WebPageBuilder::addValue(
    const String& label,
    const String& value)
{
    if (!cardOpen_)
    {
        return;
    }

    html_ += F("<div class=\"status-row\">");

    html_ += F("<span class=\"status-label\">");
    html_ += escapeHtml(label);
    html_ += F("</span>");

    html_ += F("<span class=\"status-value plain\">");
    html_ += escapeHtml(value);
    html_ += F("</span>");

    html_ += F("</div>");
}
void WebPageBuilder::addFooter(
    const String& version,
    const String& uptime)
{
    html_ += F("<footer>");

    html_ += F("<strong>Camper Sentinel ");
    html_ += escapeHtml(version);
    html_ += F("</strong>");

    html_ += F("<span>Know your camper. Anywhere.</span>");
    html_ += F("<span>Uptime ");
    html_ += escapeHtml(uptime);
    html_ += F("</span>");

    html_ += F("</footer>");

    footerAdded_ = true;
}
String WebPageBuilder::statusClass(
    StatusLevel level) const
{
    switch (level)
    {
        case StatusLevel::Normal:
            return "status-normal";

        case StatusLevel::Warning:
            return "status-warning";

        case StatusLevel::Critical:
            return "status-critical";

        case StatusLevel::Neutral:
        default:
            return "status-neutral";
    }
}

String WebPageBuilder::statusLabel(
    StatusLevel level) const
{
    switch (level)
    {
        case StatusLevel::Normal:
            return "Healthy";

        case StatusLevel::Warning:
            return "Warning";

        case StatusLevel::Critical:
            return "Critical";

        case StatusLevel::Neutral:
        default:
            return "Unknown";
    }
}