class ReplayToolbarPanelElement extends UIElement {
    constructor() {
        super(...arguments);
        this.onPanelExternalize = (e) => {
            this.m_overlayHeaderState.classList.toggle('hide', e.target.classList.contains('extern'));
        };
        
        this.onAircraftRecordButtonDown = () => {
            console.log("Recording started.");
        };
        this.onAircraftStopButtonDown = () => {
            console.log("Recording stopped.");
        };
        this.onTimelinePlayPauseButtonDown = () => {
            console.log("Play started/paused.");
        };
    
        this.setData = (data) => {
            this.m_data = data;
            if (!this.isConnected)
                return;
           
            let bCanStartRecord = true;
            let bCanStopRecord = false;
          
            this.m_buttonAircraftRecord.enable(bCanStartRecord);
            this.m_buttonAircraftStop.enable(bCanStopRecord);
        
        };
    }
    connectedCallback() {
        super.connectedCallback();
        console.log("Sky Dolly UI is connected.");        
        this.m_aircraftRecordWrapper = this.querySelector('.aircraftRecordWrapper');
        this.m_aircraftRecordWrapper.querySelector('h3').textContent = "CONTROLS";
        this.m_buttonAircraftRecord = this.querySelector('.aircraftRecord');
        this.m_buttonAircraftStop = this.querySelector('.aircraftStop');
        this.m_buttonAircraftRecord.addEventListener('OnValidate', this.onAircraftRecordButtonDown);
        this.m_buttonAircraftStop.addEventListener('OnValidate', this.onAircraftStopButtonDown);

        this.m_buttonTimelinePlayPause = this.querySelector('.timelinePlayPause');
        this.m_buttonTimelinePlayPause.addEventListener('OnValidate', this.onTimelinePlayPauseButtonDown);
        
    }
}
window.customElements.define("replay-toolbar-panel", ReplayToolbarPanelElement);
checkAutoload();
if (EDITION_MODE()) {
    document.addEventListener('DOMContentLoaded', () => {
        TemplateElement.call(document.querySelector('ingame-ui'), () => {
            document.querySelector('ingame-ui').classList.remove('panelInvisible');
            document.querySelector('ingame-ui').classList.remove('hide');
            document.querySelector('ingame-ui').classList.add('attached');
        });
        
    });
}
