import pandas as pd
import pathlib
import plotly.express as px
import plotly.io as pio
import plotly.graph_objects as go

from typing import Final

pio.templates.default = "plotly_white"


class drift_tracer:
    def __init__(self, df, is_local_clock_std, is_remote_clock_std):
        self.local_clock_suffix  = 'Std' if is_local_clock_std else 'Sys'
        self.remote_clock_suffix = 'Std' if is_remote_clock_std else 'Sys'

        us_elapsed = df['usElapsed' + self.local_clock_suffix].iloc[0]
        us_ackack_timestamp = df['usAckAckTimestamp' + self.remote_clock_suffix].iloc[0]
        self.tsbpd_base = us_elapsed - us_ackack_timestamp
        self.tsbpd_wrap_check = False
        self.TSBPD_WRAP_PERIOD = (30*1000000)
        self.MAX_TIMESTAMP = 0xFFFFFFFF # Full 32 bit (01h11m35s)

    def get_time_base(self, timestamp_us):
        carryover = 0

        if (self.tsbpd_wrap_check):
            if (timestamp_us < self.TSBPD_WRAP_PERIOD):
                carryover = self.MAX_TIMESTAMP + 1
            elif ((timestamp_us >= self.TSBPD_WRAP_PERIOD) and (timestamp_us <= (self.TSBPD_WRAP_PERIOD * 2))):
                self.tsbpd_wrap_check = False
                self.tsbpd_base += self.MAX_TIMESTAMP + 1
        elif (timestamp_us > (self.MAX_TIMESTAMP - self.TSBPD_WRAP_PERIOD)):
            self.tsbpd_wrap_check = True

        return (self.tsbpd_base + carryover)

    def calculate_drift(self, df):
        elapsed_name = 'usElapsed' + self.local_clock_suffix
        timestamp_name = 'usAckAckTimestamp' + self.remote_clock_suffix
        df_drift = df[[elapsed_name, timestamp_name, 'usDriftSampleStd']]
        df_drift = df_drift.rename(columns={elapsed_name : "usElapsed", timestamp_name : "usAckAckTimestamp", 'usDriftSampleStd' : 'usDriftSample'})

        for index, row in df_drift.iterrows():
            row['usDriftSample'] = row['usElapsed'] - (self.get_time_base(row['usAckAckTimestamp']) + row['usAckAckTimestamp'])

        df_drift['usDriftRMA'] = df_drift['usDriftSample'].ewm(com=7, adjust=False).mean()
        return df_drift



def main():
    
    df_driftlog  = pd.read_csv('win-centos-drift-trace-2.csv')

    tracer = drift_tracer(df_driftlog, False, False)
    df_drift = tracer.calculate_drift(df_driftlog)


    fig = go.Figure()
    fig.update_layout(title="Local SYS Remote SYS")
    fig.add_trace(go.Scatter(x=df_drift['usElapsed'], y=df_drift['usDriftSample'],
                    mode='lines+markers',
                    name='Drift Sample, us'))

    fig.add_trace(go.Scatter(x=df_drift['usElapsed'], y=df_drift['usDriftRMA'],
                    mode='lines+markers',
                    name='Drift RMA, us'))


    #df_driftlog['Time'] = df_driftlog['usTimestamp'] / 1000
    # fig.add_trace(go.Scatter(x=df_driftlog['usElapsedStd'], y=df_driftlog['usRTTStd'],
    #                 mode='lines+markers',
    #                 name='Instant RTT, us'))
    # fig.add_trace(go.Scatter(x=df_driftlog['usElapsedStd'], y=df_driftlog['usDriftSampleStd'],
    #                 mode='lines+markers',
    #                 name='Drift Sample, us'))

    # fig.update_layout(title='RTT and Drift Adjustment',
    #                xaxis_title='Time, ms',
    #                yaxis_title='RTT/Drift, microseconds')
    
    fig.show()


if __name__ == '__main__':
    main()
