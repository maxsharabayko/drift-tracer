import pandas as pd
import pathlib
import plotly.express as px
import plotly.io as pio
import plotly.graph_objects as go

from typing import Final

pio.templates.default = "plotly_white"


def main():
    
    df_driftlog  = pd.read_csv('mac-hp-drift-tracing-02.csv')
    df_driftlog['usDriftSampleStdActual'] = df_driftlog['usDriftSampleStd'] + df_driftlog['usOverdriftStd'].cumsum()
    df_driftlog['usDriftStdActual']       = df_driftlog['usDriftStd'] + df_driftlog['usOverdriftStd'].cumsum()

    fig = go.Figure()
    fig.update_layout(title="SRT Drift")
    fig.add_trace(go.Scatter(x=df_driftlog['Time'], y=df_driftlog['usDriftSampleStdActual'],
                    mode='lines+markers',
                    name='Drift Sample, us'))
    fig.add_trace(go.Scatter(x=df_driftlog['Time'], y=df_driftlog['usDriftStdActual'],
                    mode='lines+markers',
                    name='TSBPD Adjustment, us'))

    # fig.update_layout(title='RTT and Drift Adjustment',
    #                xaxis_title='Time, ms',
    #                yaxis_title='RTT/Drift, microseconds')
    
    fig.show()


if __name__ == '__main__':
    main()
