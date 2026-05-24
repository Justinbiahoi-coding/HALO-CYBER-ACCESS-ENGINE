import streamlit as st
import requests
import pandas as pd
import time
from datetime import datetime

#  PAGE CONFIG
st.set_page_config(
    page_title="Halo Cyber Access Engine",
    layout="wide",
    initial_sidebar_state="expanded",
)

#  GLOBAL STYLE 
st.markdown("""
<style>
/* ── Base ─────────────────────────────── */
html, body, .stApp {
    background-color: #1A1C23;
    color: #C5C9D4;
    font-family: 'Roboto Mono', 'Courier New', monospace;
}
/* ── Sidebar ──────────────────────────── */
[data-testid="stSidebar"] {
    background-color: #12141A !important;
    border-right: 1px solid #2D3040;
}
[data-testid="stSidebar"] * { color: #C5C9D4 !important; }
/* ── Top header bar ───────────────────── */
.halo-header {
    background: linear-gradient(90deg, #0F1924 0%, #1A2535 100%);
    border-bottom: 2px solid #3498DB;
    padding: 12px 24px;
    margin-bottom: 20px;
    border-radius: 4px;
    display: flex;
    align-items: center;
    gap: 12px;
}
.halo-header h1 {
    color: #FFFFFF !important;
    font-size: 1.4rem !important;
    margin: 0 !important;
    letter-spacing: 2px;
}
.halo-badge {
    background: #3498DB;
    color: #fff !important;
    font-size: 0.65rem;
    padding: 2px 8px;
    border-radius: 3px;
    font-weight: bold;
    letter-spacing: 1px;
}
/* ── Section title ───────────────────── */
.section-title {
    color: #3498DB !important;
    font-size: 0.75rem;
    letter-spacing: 3px;
    text-transform: uppercase;
    margin-bottom: 4px;
    border-bottom: 1px solid #2D3040;
    padding-bottom: 6px;
}
/* ── Metric cards ─────────────────────── */
.metric-row { display: flex; gap: 12px; margin-bottom: 20px; }
.metric-card {
    flex: 1;
    background: #1E2130;
    border: 1px solid #2D3040;
    border-left: 3px solid #3498DB;
    border-radius: 4px;
    padding: 14px 18px;
}
.metric-card.green  { border-left-color: #2ECC71; }
.metric-card.orange { border-left-color: #E67E22; }
.metric-card.red    { border-left-color: #E74C3C; }
.metric-card .label { font-size: 0.65rem; color: #7A8199; letter-spacing: 2px; text-transform: uppercase; }
.metric-card .value { font-size: 1.4rem; color: #FFFFFF; font-weight: bold; margin-top: 4px; }
.metric-card .sub   { font-size: 0.7rem; color: #7A8199; margin-top: 2px; }
/* ── Search bar & Inputs ──────────────── */
.stTextInput > div > div > input, .stNumberInput > div > div > input {
    background-color: #0F1117 !important;
    border: 1px solid #3498DB !important;
    border-radius: 3px !important;
    color: #C5C9D4 !important;
    font-family: 'Roboto Mono', monospace;
    font-size: 0.85rem;
}
/* ── Buttons ──────────────────────────── */
.stButton > button {
    background-color: #3498DB !important;
    color: #FFFFFF !important;
    border: none !important;
    border-radius: 3px !important;
    font-weight: bold !important;
    letter-spacing: 1px !important;
    font-size: 0.75rem !important;
    padding: 8px 20px !important;
    transition: all 0.2s;
}
.stButton > button:hover {
    background-color: #2980B9 !important;
    box-shadow: 0 0 12px rgba(52,152,219,0.4) !important;
}
/* ── Dataframe ────────────────────────── */
[data-testid="stDataFrame"] {
    border: 1px solid #2D3040;
    border-radius: 4px;
}
/* ── Radio & Select (sidebar nav) ─────── */
[data-testid="stRadio"] label, [data-testid="stSelectbox"] label {
    color: #C5C9D4 !important;
    font-size: 0.8rem !important;
    letter-spacing: 1px;
}
/* ── Alert boxes ─────────────────────── */
.alert-box {
    background: #1E1A0F;
    border: 1px solid #E67E22;
    border-left: 4px solid #E67E22;
    border-radius: 4px;
    padding: 12px 16px;
    margin: 6px 0;
    font-size: 0.8rem;
}
.alert-box.critical {
    background: #2A1718;
    border-color: #E74C3C;
}
.alert-box .al-title { font-weight: bold; }
.alert-box.critical .al-title { color: #E74C3C; }
.alert-box:not(.critical) .al-title { color: #E67E22; }
.alert-box .al-body  { color: #C5C9D4; margin-top: 4px; }
/* ── Divider ─────────────────────────── */
hr { border-color: #2D3040 !important; }
/* ── Spinner text ───────────────────── */
.stSpinner > div { color: #3498DB !important; }
/* ── Info / warning / error ──────────── */
.stAlert { border-radius: 4px !important; font-size: 0.8rem !important; }
</style>
""", unsafe_allow_html=True)

#  HELPER FUNCTIONS
API_BASE = "http://localhost:8080"

def api_get(path: str):
    try:
        r = requests.get(API_BASE + path, timeout=15)
        r.raise_for_status()
        return r.json(), None
    except requests.exceptions.ConnectionError:
        return None, "CRITICAL: Cannot connect to C++ Engine on port 8080. Verify the backend process is running."
    except requests.exceptions.Timeout:
        return None, "TIMEOUT: Request to engine timed out."
    except Exception as e:
        return None, f"SYSTEM ERROR: {e}"

def fmt_epoch(ts):
    try:
        return datetime.utcfromtimestamp(int(ts)).strftime("%Y-%m-%d %H:%M:%S")
    except:
        return str(ts)

def render_header(title: str, badge: str = "LIVE"):
    st.markdown(f"""
    <div class="halo-header">
        <h1>HALO CYBER ACCESS ENGINE &nbsp;·&nbsp; {title}</h1>
        <span class="halo-badge">{badge}</span>
    </div>""", unsafe_allow_html=True)

def render_metric(label, value, sub="", color=""):
    return f"""<div class="metric-card {color}">
        <div class="label">{label}</div>
        <div class="value">{value}</div>
        <div class="sub">{sub}</div>
    </div>"""

def connection_error(msg):
    st.error(msg)
    st.code("# Ensure C++ Engine is initialized:\n./release/halo_engine", language="bash")

#  SIDEBAR
with st.sidebar:
    st.markdown('<p style="font-size:1.1rem;font-weight:bold;color:#FFFFFF;letter-spacing:2px">HALO SOC CONSOLE</p>', unsafe_allow_html=True)
    st.markdown("---")

    menu = st.radio("NAVIGATION", [
        "Overview: Top 10",
        "User Journey",
        "Resource Tracker",
        "Anomaly Radar",
        "Data Quality",
    ], label_visibility="collapsed")

    st.markdown("---")
    st.markdown('<p class="section-title" style="font-size:0.65rem;">TIME RANGE FILTER</p>', unsafe_allow_html=True)
    
    # Time Input Toggle
    time_mode = st.selectbox("INPUT FORMAT", ["Standard Date", "Epoch Timestamp"], label_visibility="collapsed")

    if time_mode == "Standard Date":
        # Standard date retains the 2-column layout because dates are short
        col_a, col_b = st.columns(2)
        with col_a:
            start_date = st.date_input("From", value=pd.Timestamp("2024-04-13"))
        with col_b:
            end_date = st.date_input("To", value=pd.Timestamp("2025-04-15"))
        start_epoch = int(pd.Timestamp(start_date).timestamp())
        end_epoch   = int(pd.Timestamp(end_date).timestamp())
    else:
        # Epoch timestamp uses vertical layout (no columns) to avoid UI truncation
        start_epoch = st.number_input("From (Epoch)", value=1712966400, step=1, format="%d")
        end_epoch = st.number_input("To (Epoch)", value=1744675200, step=1, format="%d")
        start_epoch = int(start_epoch)
        end_epoch = int(end_epoch)

    st.markdown(f"""
    <div style="background:#0F1117;border:1px solid #2D3040;border-radius:3px;padding:8px 10px;font-size:0.75rem;color:#7A8199;font-family:monospace;margin-top:10px;">
    <b style="color:#3498DB">START_TS</b>: {start_epoch}<br>
    <b style="color:#3498DB">END_TS&nbsp;&nbsp;</b>: {end_epoch}
    </div>""", unsafe_allow_html=True)

    st.markdown("---")
    st.markdown('<p style="font-size:0.65rem;color:#7A8199;letter-spacing:1px">SERVER: LOCALHOST:8080</p>', unsafe_allow_html=True)

#  MODULE 1 — OVERVIEW / TOP 10
if "Overview" in menu:
    render_header("OVERVIEW DASHBOARD")

    m1 = render_metric("DATASET",  "1,500,000", "log entries loaded", "")
    m2 = render_metric("ENGINE",   "ONLINE",    "C++ HaloEngine v1",  "green")
    m3 = render_metric("RAM USAGE","~120 MB",   "memory pool active", "orange")
    m4 = render_metric("API P50",  "< 2 ms",    "query latency",      "")
    st.markdown(f'<div class="metric-row">{m1}{m2}{m3}{m4}</div>', unsafe_allow_html=True)

    st.markdown('<p class="section-title">TOP 10 MOST-ACCESSED RESOURCES</p>', unsafe_allow_html=True)

    if st.button("EXECUTE QUERY"):
        with st.spinner("Executing query on C++ engine..."):
            data, err = api_get(f"/api/top?start={start_epoch}&end={end_epoch}")

        if err:
            connection_error(err)
        elif not data:
            st.warning("No resources found in the specified time range.")
        else:
            df = pd.DataFrame(data)
            max_hits = int(df["count"].max())

            col_l, col_r = st.columns([1.3, 1])

            with col_l:
                st.markdown('<p class="section-title">ACCESS FREQUENCY TABLE</p>', unsafe_allow_html=True)
                st.dataframe(
                    df,
                    column_config={
                        "resource_id": st.column_config.TextColumn("Resource ID", width="medium"),
                        "count": st.column_config.ProgressColumn(
                            "Access Count",
                            format="%d",
                            min_value=0,
                            max_value=max_hits,
                        ),
                    },
                    hide_index=True,
                    use_container_width=True,
                )

            with col_r:
                st.markdown('<p class="section-title">FREQUENCY DISTRIBUTION</p>', unsafe_allow_html=True)
                chart_df = df.set_index("resource_id")
                st.bar_chart(chart_df, color="#3498DB", use_container_width=True)

            total_hits = int(df["count"].sum())
            st.markdown("---")
            s1 = render_metric("TOTAL ACCESSES IN RANGE", f"{total_hits:,}", f"{fmt_epoch(start_epoch)} → {fmt_epoch(end_epoch)}")
            s2 = render_metric("TOP RESOURCE", df.iloc[0]["resource_id"], f"{df.iloc[0]['count']:,} hits", "green")
            s3 = render_metric("RESOURCES TRACKED", str(len(df)), "unique IDs in top 10")
            st.markdown(f'<div class="metric-row">{s1}{s2}{s3}</div>', unsafe_allow_html=True)

#  MODULE 2 — USER JOURNEY
elif "User Journey" in menu:
    render_header("USER JOURNEY INVESTIGATION")

    st.markdown('<p class="section-title">TARGET USER SEARCH</p>', unsafe_allow_html=True)

    col1, col2 = st.columns([3, 1])
    with col1:
        target_user = st.text_input("User ID", value="U03649", placeholder="e.g. U03649", label_visibility="collapsed")
    with col2:
        run = st.button("TRACK USER")

    if run and target_user:
        with st.spinner(f"Compiling journey for {target_user}..."):
            data, err = api_get(f"/api/user?id={target_user}&start={start_epoch}&end={end_epoch}")

        if err:
            connection_error(err)
        elif not data:
            st.warning(f"No activity logs found for user {target_user} in the specified time range.")
        else:
            df = pd.DataFrame(data)
            df["datetime"] = pd.to_datetime(df["timestamp"], unit="s")

            devices   = df["device_id"].nunique()
            apps      = df["app_id"].nunique()
            resources = df["resource_id"].nunique()
            span_h    = round((df["timestamp"].max() - df["timestamp"].min()) / 3600, 1)

            s1 = render_metric("EVENTS",    f"{len(df):,}",  "in time range")
            s2 = render_metric("DEVICES",   str(devices),    "unique devices",   "orange" if devices > 3 else "")
            s3 = render_metric("APPS",      str(apps),       "unique apps")
            s4 = render_metric("RESOURCES", str(resources),  "unique resources")
            s5 = render_metric("SPAN",      f"{span_h}h",    "activity window")
            st.markdown(f'<div class="metric-row">{s1}{s2}{s3}{s4}{s5}</div>', unsafe_allow_html=True)

            tab1, tab2, tab3 = st.tabs(["Event Log", "Timeline", "Breakdown"])

            with tab1:
                st.markdown('<p class="section-title">ACCESS SEQUENCE (DEVICE → APP → RESOURCE)</p>', unsafe_allow_html=True)
                display_df = df[["datetime", "timestamp", "device_id", "app_id", "resource_id"]].copy()
                display_df.rename(columns={
                    "datetime": "Date & Time (UTC)",
                    "timestamp": "Epoch TS",
                    "device_id": "Device ID",
                    "app_id": "App ID",
                    "resource_id": "Resource ID"
                }, inplace=True)
                st.dataframe(display_df, use_container_width=True, hide_index=True)

            with tab2:
                st.markdown('<p class="section-title">ACTIVITY TIMELINE</p>', unsafe_allow_html=True)
                df["hour"] = df["datetime"].dt.floor("h")
                timeline = df.groupby("hour").size().reset_index(name="events")
                timeline = timeline.set_index("hour")
                st.area_chart(timeline, color="#3498DB", use_container_width=True)

            with tab3:
                st.markdown('<p class="section-title">TOP ACCESSED RESOURCES & APPS</p>', unsafe_allow_html=True)
                col_a, col_b = st.columns(2)
                with col_a:
                    res_counts = df["resource_id"].value_counts().reset_index()
                    res_counts.columns = ["Resource ID", "Hit Count"]
                    st.dataframe(res_counts, hide_index=True, use_container_width=True)
                with col_b:
                    app_counts = df["app_id"].value_counts().reset_index()
                    app_counts.columns = ["App ID", "Hit Count"]
                    st.dataframe(app_counts, hide_index=True, use_container_width=True)

            st.markdown("---")
            st.markdown('<p class="section-title">SYSTEM ANOMALY FLAGS</p>', unsafe_allow_html=True)
            hints = []
            if devices > 4:
                hints.append(("HIGH DEVICE COUNT", f"Account accessed from {devices} distinct devices. Evaluate for credential sharing or compromise."))
            if span_h > 0 and len(df) / span_h > 500:
                hints.append(("ELEVATED ACTIVITY RATE", f"Sustained {len(df)/span_h:.0f} events/hour. Potential automated script behavior."))
            
            if not hints:
                st.success("STATUS CLEAR: No standard anomalies detected for this user in the selected range.")
            else:
                for title, body in hints:
                    st.markdown(f'<div class="alert-box"><div class="al-title">WARNING: {title}</div><div class="al-body">{body}</div></div>', unsafe_allow_html=True)

#  MODULE 3 — RESOURCE TRACKER
elif "Resource Tracker" in menu:
    render_header("RESOURCE ACCESS TRACKER")

    st.markdown('<p class="section-title">TARGET RESOURCE SEARCH</p>', unsafe_allow_html=True)

    col1, col2 = st.columns([3, 1])
    with col1:
        target_res = st.text_input("Resource ID", value="R03922", placeholder="e.g. R03922", label_visibility="collapsed")
    with col2:
        run = st.button("AUDIT RESOURCE")

    if run and target_res:
        with st.spinner(f"Compiling access logs for {target_res}..."):
            data, err = api_get(f"/api/resource?id={target_res}&start={start_epoch}&end={end_epoch}")

        if err:
            connection_error(err)
        elif not data:
            st.warning(f"No access records found for resource {target_res} in the specified time range.")
        else:
            df = pd.DataFrame(data)
            df["datetime"] = pd.to_datetime(df["timestamp"], unit="s")

            users   = df["user_id"].nunique()
            devices = df["device_id"].nunique()
            apps    = df["app_id"].nunique()

            s1 = render_metric("TOTAL ACCESSES", f"{len(df):,}", "in time range")
            s2 = render_metric("UNIQUE USERS",   str(users),    "distinct accounts",  "orange" if users > 20 else "")
            s3 = render_metric("UNIQUE DEVICES", str(devices),  "distinct devices")
            s4 = render_metric("UNIQUE APPS",    str(apps),     "distinct apps")
            st.markdown(f'<div class="metric-row">{s1}{s2}{s3}{s4}</div>', unsafe_allow_html=True)

            tab1, tab2, tab3 = st.tabs(["Access Log", "Access Timeline", "Top Users"])

            with tab1:
                st.markdown('<p class="section-title">ACCESS LOG (USER → DEVICE → APP)</p>', unsafe_allow_html=True)
                display_df = df[["datetime", "timestamp", "user_id", "device_id", "app_id"]].copy()
                display_df.rename(columns={
                    "datetime": "Date & Time (UTC)",
                    "timestamp": "Epoch TS",
                    "user_id": "User ID",
                    "device_id": "Device ID",
                    "app_id": "App ID"
                }, inplace=True)
                st.dataframe(display_df, use_container_width=True, hide_index=True)

            with tab2:
                st.markdown('<p class="section-title">ACCESS FREQUENCY OVER TIME</p>', unsafe_allow_html=True)
                df["hour"] = df["datetime"].dt.floor("h")
                timeline = df.groupby("hour").size().reset_index(name="accesses")
                timeline = timeline.set_index("hour")
                st.area_chart(timeline, color="#E74C3C", use_container_width=True)

            with tab3:
                st.markdown('<p class="section-title">MOST FREQUENT ACCESSORS</p>', unsafe_allow_html=True)
                col_a, col_b = st.columns(2)
                with col_a:
                    top_users = df["user_id"].value_counts().reset_index().head(20)
                    top_users.columns = ["User ID", "Access Count"]
                    st.dataframe(top_users, hide_index=True, use_container_width=True)
                with col_b:
                    top_devices = df["device_id"].value_counts().reset_index().head(20)
                    top_devices.columns = ["Device ID", "Access Count"]
                    st.dataframe(top_devices, hide_index=True, use_container_width=True)

            st.markdown("---")
            st.markdown('<p class="section-title">SYSTEM ANOMALY FLAGS</p>', unsafe_allow_html=True)
            hints = []
            if users > 50:
                hints.append(("WIDE ACCESS DISTRIBUTON", f"{users} distinct users accessed this resource. Verify if resource classification allows broad access."))
            
            if not hints:
                st.success("STATUS CLEAR: No standard anomalies detected for this resource in the selected range.")
            else:
                for title, body in hints:
                    st.markdown(f'<div class="alert-box"><div class="al-title">WARNING: {title}</div><div class="al-body">{body}</div></div>', unsafe_allow_html=True)

#  MODULE 4 — ANOMALY RADAR
elif "Anomaly Radar" in menu:
    render_header("ANOMALY DETECTION RADAR", badge="MODULE 4")

    st.markdown('<p class="section-title">DETECTION RULES DEPLOYMENT STATUS</p>', unsafe_allow_html=True)

    modules = [
        ("CRITICAL", "BRUTE FORCE DETECTION",     "Continuous FAILED_LOGIN attempts triggering threshold alert", "Planned"),
        ("CRITICAL", "MULTI-DEVICE ANOMALY",      "User logs in from N+ devices within a restricted time window", "Planned"),
        ("CRITICAL", "GEO-HOPPING ALERT",         "Impossible travel sequence: location jumps exceed flight time metrics", "Planned"),
        ("WARNING",  "OFF-HOURS ACCESS",          "System activity detected outside 08:00–18:00 local business hours", "Planned"),
        ("WARNING",  "SESSION HIJACK RISK",       "ADMIN_ACTION followed by mass DOWNLOAD execution in identical session", "Planned"),
        ("WARNING",  "LONG SESSION ANOMALY",      "Session duration exceeds standard operational threshold (>12h)", "Planned"),
        ("WARNING",  "SILENT-THEN-BURST",         "User inactive >7 days, followed by sudden high-volume event stream", "Advanced"),
        ("WARNING",  "CREDENTIAL STUFFING",       "Multiple FAILED_LOGINs from scattered IPs culminating in SUCCESS", "Advanced"),
    ]

    for severity, title, desc, tag in modules:
        box_class = "alert-box critical" if severity == "CRITICAL" else "alert-box"
        tag_color = "#E67E22" if tag == "Advanced" else "#3498DB"
        
        st.markdown(f"""
        <div class="{box_class}">
          <div style="display:flex;align-items:center;gap:10px">
            <span class="al-title" style="flex:1">[{severity}] {title}</span>
            <span style="font-size:0.65rem;background:{tag_color};color:#fff;padding:2px 8px;border-radius:3px;font-weight:bold;">{tag.upper()}</span>
          </div>
          <div class="al-body">{desc}</div>
        </div>""", unsafe_allow_html=True)

    st.markdown("---")
    st.info("NOTE: Telemetry for these modules requires the integration of secondary C++ Hash Tables (e.g., Device Indexing) scheduled for Phase 4 deployment.")
#  MODULE 5 — DATA QUALITY
elif "Data Quality" in menu:
    render_header("DATA QUALITY REPORT", badge="INGEST")

    with st.spinner("Fetching quality report from engine..."):
        data, err = api_get("/api/quality")

    if err:
        connection_error(err)
    else:
        loaded   = data.get("loaded", 0)
        skipped  = data.get("total_skipped", 0)
        total    = loaded + skipped
        pct_ok   = f"{loaded/total*100:.1f}%" if total > 0 else "N/A"

        mal  = data.get("malformed",  {})
        inv  = data.get("invalid",    {})
        dup  = data.get("duplicate",  {})

        # ── Summary metrics ────────────────────────────────────────────────
        m1 = render_metric("TOTAL ROWS READ",    f"{total:,}",              "from CSV file")
        m2 = render_metric("LOADED",             f"{loaded:,}",             "valid rows", "green")
        m3 = render_metric("SKIPPED",            f"{skipped:,}",            "invalid / filtered", "red" if skipped > 0 else "")
        m4 = render_metric("ACCEPTANCE RATE",    pct_ok,                    "rows passed validation")
        st.markdown(f'<div class="metric-row">{m1}{m2}{m3}{m4}</div>', unsafe_allow_html=True)

        # ── Per-category summary ───────────────────────────────────────────
        st.markdown('<p class="section-title">SKIP CATEGORY BREAKDOWN</p>', unsafe_allow_html=True)
        c1, c2, c3 = st.columns(3)
        with c1:
            st.markdown(f"""
            <div class="metric-card red">
                <div class="label">MALFORMED</div>
                <div class="value">{mal.get('total', 0):,}</div>
                <div class="sub">Wrong column count</div>
            </div>""", unsafe_allow_html=True)
        with c2:
            st.markdown(f"""
            <div class="metric-card orange">
                <div class="label">INVALID</div>
                <div class="value">{inv.get('total', 0):,}</div>
                <div class="sub">Bad event / location / ID / timestamp</div>
            </div>""", unsafe_allow_html=True)
        with c3:
            st.markdown(f"""
            <div class="metric-card">
                <div class="label">DUPLICATE</div>
                <div class="value">{dup.get('total', 0):,}</div>
                <div class="sub">Same user + timestamp + event</div>
            </div>""", unsafe_allow_html=True)

        st.markdown("---")

        # ── Sample tables ──────────────────────────────────────────────────
        tab_mal, tab_inv, tab_dup = st.tabs([
            f"🔴 Malformed ({mal.get('total', 0):,})",
            f"🟠 Invalid ({inv.get('total', 0):,})",
            f"⚪ Duplicate ({dup.get('total', 0):,})",
        ])

        def render_sample_table(tab, samples, empty_msg):
            with tab:
                if not samples:
                    st.success(empty_msg)
                    return
                df = pd.DataFrame(samples)
                df.index = df.index + 1
                df.columns = ["Raw Row", "Reason"]
                st.caption(f"Showing {len(samples)} sample(s) — engine stores up to 300 per category.")
                st.dataframe(df, use_container_width=True)

        render_sample_table(tab_mal, mal.get("samples", []),
                            "✅ No malformed rows detected.")
        render_sample_table(tab_inv, inv.get("samples", []),
                            "✅ No invalid rows detected.")
        render_sample_table(tab_dup, dup.get("samples", []),
                            "✅ No duplicate rows detected.")
