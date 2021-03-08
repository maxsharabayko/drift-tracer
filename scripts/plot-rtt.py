import pandas as pd
import pathlib
import plotly.express as px
import plotly.io as pio
import plotly.graph_objects as go

pio.templates.default = "plotly_white"



def main():
    
    df_driftlog  = pd.read_csv('hp-mac-drift-tracing-01.csv')

    fig = go.Figure()
    fig.update_layout(title="RTT")
    fig.add_trace(go.Scatter(x=df_driftlog['usElapsedStd'], y=df_driftlog['usRTTStd'],
                    mode='lines+markers',
                    name='Instant RTT steady, us'))

    fig.add_trace(go.Scatter(x=df_driftlog['usElapsedStd'], y=df_driftlog['usRTTStdRma'],
                    mode='lines+markers',
                    name='RTT RMA steady, us'))

    fig.show()


if __name__ == '__main__':
    main()
