import pandas as pd
import pathlib
import plotly.express as px
import plotly.io as pio
import plotly.graph_objects as go

import click

pio.templates.default = "plotly_white"

@click.command()
@click.argument(
    'filepath',
    type=click.Path(exists=True)
)
def main(filepath):
    
    df_driftlog  = pd.read_csv(filepath)
    df_driftlog['usDriftSampleStdActual'] = df_driftlog['usDriftSampleStd'] + df_driftlog['usOverdriftStd'].cumsum()
    df_driftlog['usDriftStdActual']       = df_driftlog['usDriftStd'] + df_driftlog['usOverdriftStd'].cumsum()
    df_driftlog['sTime'] = df_driftlog['usElapsedStd'] / 1000000

    fig = go.Figure()
    fig.update_layout(title="SRT Drift")
    fig.add_trace(go.Scatter(x=df_driftlog['sTime'], y=df_driftlog['usDriftSampleStdActual'],
                    mode='lines+markers',
                    name='Drift Sample, us'))
    fig.add_trace(go.Scatter(x=df_driftlog['sTime'], y=df_driftlog['usDriftStdActual'],
                    mode='lines+markers',
                    name='TSBPD Adjustment, us'))

    # fig.update_layout(title='RTT and Drift Adjustment',
    #                xaxis_title='Time, ms',
    #                yaxis_title='RTT/Drift, microseconds')
    
    fig.show()


if __name__ == '__main__':
    main()
