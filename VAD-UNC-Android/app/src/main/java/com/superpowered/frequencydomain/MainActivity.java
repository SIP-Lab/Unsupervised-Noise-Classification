package com.superpowered.frequencydomain;

import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.media.AudioManager;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;

import org.w3c.dom.Text;

import java.io.File;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    private ActivityInference activityInference;
    private DecimalFormat decimalFormat;
    private Handler handler = new Handler();
    private TextView blanktext, processingTime, NoisePower, classification, noiseLabel, speechLabel;
    private MovingAverageBuffer averageBuffer, timeBuffer;
    private int CNNUpdate = 62, UIUpdate = 1000;//it was 200
    private String samplerateString = null, buffersizeString = null;
    private Button buttonStart, buttonStop,readFileButton;

    int IsNoiseDetected = 1;
    String folderName = Environment.getExternalStorageDirectory().toString() + "/VAD_UNC/";
    String StoreFeaturesFileName, HybridSVDDFileName, HybridArt2FileName, audioFileName; // for hybrid classification
    TextView statusView;
    Switch storeFeaturesSwitch, SavingClassificationDataSwitch, HybridClassificationSwitch ; //playAudioSwitch
    EditText  FeatAvgBuffVal, NewClusterCreaBuffVal, DecisionSmoothingBuffVal , VigilanceVal1 ,VigilanceVal2; //sigmaVal ,fractionRejVal,ChunkSizeVal ; //FeatAvgBuffVal ,decisionRate, DecisionRateVal
    SharedPreferences prefs;
    SharedPreferences.Editor prefEdit;
    public static final String appPreferences = "appPrefs";

    final Runnable r = new Runnable() {
        @Override
        public void run() {
            long startTime = System.currentTimeMillis();
            float prob[] = activityInference.getActivityProb(GetMelImage()); // here we fed the images to CNN and get probabilities
            averageBuffer.addDatum(prob[1]); // here we use moving average to get result of CNN after every 11 frames
            timeBuffer.addDatum((System.currentTimeMillis() - startTime));

            handler.postDelayed(this,CNNUpdate);
        }
    };

    final Runnable timeUpdate = new Runnable() {
        @Override
        public void run() {
//            blanktext.setText("Result " + decimalFormat.format(averageBuffer.movingAverage));
            processingTime.setText(decimalFormat.format(getProcessingTime()) + " ms");
            NoisePower.setText( new DecimalFormat("##").format(getdbPower()) + " dB");

            if (Math.round(averageBuffer.movingAverage) == 0){ // noise detected
                //classification.setText("Noise");
                noiseLabel.setBackgroundColor(Color.RED);
                speechLabel.setBackgroundColor(Color.WHITE);
                IsNoiseDetected = 1;
                getTime(); // show the results of classification
            }
            else{
                //classification.setText("Speech in Noise"); // speech + noise detected
                noiseLabel.setBackgroundColor(Color.WHITE);
                speechLabel.setBackgroundColor(Color.GREEN);
                IsNoiseDetected = 0;
            }

            //handler.postDelayed(this, 1000);
            handler.postDelayed(this, UIUpdate);
        }
    };



    public void setUserDefaults(){

        if(prefs.getBoolean("Initialized", false)) {

            prefEdit.putString("Decision Smoothing Time (sec.)", DecisionSmoothingBuffVal.getText().toString());
            prefEdit.putString("New Class Creation Time (sec.)",NewClusterCreaBuffVal.getText().toString());
            prefEdit.putString("#Frame Feature Averaging", FeatAvgBuffVal.getText().toString());
            prefEdit.putBoolean("Hybrid Classification", HybridClassificationSwitch.isChecked());
            prefEdit.putBoolean("Saving Class Representatives", SavingClassificationDataSwitch.isChecked());
            prefEdit.putBoolean("Saving Features and Classification Results", storeFeaturesSwitch.isChecked());
            //prefEdit.putString("Sigma",sigmaVal.getText().toString());
            //prefEdit.putString("Fraction Rejection", fractionRejVal.getText().toString());
            //prefEdit.putString("Classifier Mode", ClassifierModeVal.getText().toString());
            //prefEdit.putString("Feature Extraction Mode", FeatureExtractionModeVal.getText().toString());
            prefEdit.putString("Vigilance 1", VigilanceVal1.getText().toString());
            prefEdit.putString(",   Vigilance 2", VigilanceVal2.getText().toString());
            //prefEdit.putBoolean("Play Audio", playAudioSwitch.isChecked());
            //prefEdit.putString("#Decisions in Chunk", ChunkSizeVal.getText().toString());
            prefEdit.apply();
        }
        else {
            prefEdit.putBoolean("Initialized", true);
            prefEdit.putString("Decision Smoothing Time (sec.)", "3");
            prefEdit.putString("New Class Creation Time (sec.)", "5");
            prefEdit.putString("#Frame Feature Averaging", "80");
            prefEdit.putBoolean("Hybrid Classification", false);
            prefEdit.putBoolean("Saving Class Representatives", false);
            prefEdit.putBoolean("Saving Features and Classification Results", false);
            //prefEdit.putString("Sigma", "2.0");
            //prefEdit.putString("Fraction Rejection", "0.01");
            //prefEdit.putString("Classifier Mode", "2");
            //prefEdit.putString("Feature Extraction Mode", "2");
            prefEdit.putString("Vigilance 1", "0.01");
            prefEdit.putString(",   Vigilance 2", "0.01");
            //prefEdit.putBoolean("Play Audio", false);
            //prefEdit.putString("#Decisions in Chunk", "10");
            prefEdit.apply();
        }
    }
    public void loadUserDefaults(){
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        if(prefs.getBoolean("Initialized", false)) {
            DecisionSmoothingBuffVal.setText(prefs.getString("Decision Smoothing Time (sec.)", "3"));
            NewClusterCreaBuffVal.setText(prefs.getString("New Class Creation Time (sec.)", "5"));
            FeatAvgBuffVal.setText(prefs.getString("#Frame Feature Averaging", "80"));
            HybridClassificationSwitch.setChecked(prefs.getBoolean("Hybrid Classification", false));
            SavingClassificationDataSwitch.setChecked(prefs.getBoolean("Saving Class Representatives", false));
            storeFeaturesSwitch.setChecked(prefs.getBoolean("Saving Features and Classification Results", false));
            //sigmaVal.setText(prefs.getString("Sigma", "2.0"));
            //fractionRejVal.setText(prefs.getString("Fraction Rejection", "0.01"));
            //ClassifierModeVal.setText(prefs.getString("Classifier Mode", "2"));
            //FeatureExtractionModeVal.setText(prefs.getString("Feature Extraction Mode", "2"));
            VigilanceVal1.setText(prefs.getString("Vigilance 1", "0.01"));
            VigilanceVal2.setText(prefs.getString(",   Vigilance 2", "0.01"));
            //playAudioSwitch.setChecked(prefs.getBoolean("Play Audio", false));
            //ChunkSizeVal.setText(prefs.getString("#Decisions in Chunk", "10"));
        }
    }

    public void enableButtons(){
        HybridClassificationSwitch.setEnabled(true);
        SavingClassificationDataSwitch.setEnabled(true);
        storeFeaturesSwitch.setEnabled(true);
        DecisionSmoothingBuffVal.setEnabled(true);
        NewClusterCreaBuffVal.setEnabled(true);
        FeatAvgBuffVal.setEnabled(true);
        //sigmaVal.setEnabled(true);
        //fractionRejVal.setEnabled(true);
        //ClassifierModeVal.setEnabled(true);
        //FeatureExtractionModeVal.setEnabled(true);
        VigilanceVal1.setEnabled(true);
        VigilanceVal2.setEnabled(true);
        readFileButton.setEnabled(true);
        //playAudioSwitch.setEnabled(true);
        //ChunkSizeVal.setEnabled(true);
    }

    public void disableButtons(){
        HybridClassificationSwitch.setEnabled(false);
        SavingClassificationDataSwitch.setEnabled(false);
        storeFeaturesSwitch.setEnabled(false);
        DecisionSmoothingBuffVal.setEnabled(false);
        NewClusterCreaBuffVal.setEnabled(false);
        FeatAvgBuffVal.setEnabled(false);
        //sigmaVal.setEnabled(false);
        //fractionRejVal.setEnabled(false);
        //ClassifierModeVal.setEnabled(false);
        //FeatureExtractionModeVal.setEnabled(false);
        VigilanceVal1.setEnabled(false);
        VigilanceVal2.setEnabled(false);
        readFileButton.setEnabled(true);
        //playAudioSwitch.setEnabled(false);
        //ChunkSizeVal.setEnabled(false);
    }
    private void initializeVariables(int samplerate, int buffersize) {

        activityInference = new ActivityInference(getApplicationContext());
        decimalFormat     = new DecimalFormat("0.00");
//        blanktext         = (TextView) findViewById(R.id.blanktext);
        processingTime    = (TextView) findViewById(R.id.processingTime);
        NoisePower        = (TextView) findViewById(R.id.NoisePower);
//        classification    = (TextView) findViewById(R.id.classification);

        speechLabel       = (TextView) findViewById(R.id.speechLabel);
        noiseLabel        = (TextView) findViewById(R.id.noiseLabel);

        averageBuffer     = new MovingAverageBuffer(11); //here we define how many frames needs to detect speech+noise or noise
        timeBuffer        = new MovingAverageBuffer(samplerate/buffersize);

        buttonStart       = (Button) findViewById(R.id.buttonStart);
        buttonStop        = (Button) findViewById(R.id.buttonStop);
        readFileButton    = (Button) findViewById(R.id.buttonRead);

        buttonStop.setBackgroundColor(Color.LTGRAY);

        statusView        = (TextView) findViewById(R.id.statusView); // for showing results of noise classification
        HybridClassificationSwitch = (Switch) findViewById(R.id.switchHybridClassification);
        SavingClassificationDataSwitch = (Switch) findViewById(R.id.switchSavingClassificationData);
        DecisionSmoothingBuffVal = (EditText) findViewById(R.id.DecisionSmoothingBuffVal);
        NewClusterCreaBuffVal = (EditText) findViewById(R.id.NewClusterCreaBuffVall);
        FeatAvgBuffVal = (EditText) findViewById(R.id.FeatAvgBuffVal);
        //sigmaVal = (EditText) findViewById(R.id.sigmaVal);
        //fractionRejVal = (EditText) findViewById(R.id.fractionRejVal);
        //ClassifierModeVal = (EditText) findViewById(R.id.ClassifierModeVal);
        //FeatureExtractionModeVal = (EditText) findViewById(R.id.FeatureExtractionModeVal);
        VigilanceVal1 = (EditText) findViewById(R.id.VigilanceVal1);
        VigilanceVal2 = (EditText) findViewById(R.id.VigilanceVal2);
        //playAudioSwitch = (Switch) findViewById(R.id.switchPlayAudio);
        //ChunkSizeVal = (EditText) findViewById(R.id.ChunkSizeVal);
        storeFeaturesSwitch = (Switch) findViewById(R.id.switchStoreFeatures);
    }

    public void buttonStartClick(View view){
        setUserDefaults();
        disableButtons();
        int bufferSize; //should be 600
        int StepSize = 25;

        // if FeatAvgBuffVal = 80, OverlapSize = 12.5 ms, NewClusterCreaBuffVal = 3 sec, then DigitNewClusterCreaBuff = 3.
        //int FeatAvgBuffVal = 80;
        int DigitNewClusterCreaBuff = (int)((Float.parseFloat(NewClusterCreaBuffVal.getText().toString()) * 1000)/( (StepSize/2) * Integer.parseInt(FeatAvgBuffVal.getText().toString()) ) ); //
        int DigitDecisionSmoothBuff = (int)((Float.parseFloat(DecisionSmoothingBuffVal.getText().toString()) * 1000)/( (StepSize/2) * Integer.parseInt(FeatAvgBuffVal.getText().toString()) ) ); //Integer.parseInt(FeatAvgBuffVal.getText().toString())
        bufferSize = (int) ((48000 * StepSize)/(2*1000)); //getSamplingRate(), Float.parseFloat(OverlapSizeVal.getText().toString()

        statusView.append("\nRecording Started\n");

        if(storeFeaturesSwitch.isChecked()){
            StoreFeaturesFileName = folderName + new SimpleDateFormat("yyyy_MM_dd_HH_mm_ss").format(new Date()) + ".txt";
            Log.d("Filename",StoreFeaturesFileName);
        }

        if(HybridClassificationSwitch.isChecked() || SavingClassificationDataSwitch.isChecked()) {

            HybridSVDDFileName = folderName + "HybridSVDD_clusterParameters" + ".dat";
            Log.d("Filename", HybridSVDDFileName);

            HybridArt2FileName = folderName + "HybridArt2Fusion_clusterParameters" + ".dat";

            /*if (Integer.parseInt(FeatureExtractionModeVal.getText().toString()) == 1){
                HybridArt2FileName = folderName + "HybridArt2Subband_clusterParameters" + ".dat";}
                else if (Integer.parseInt(FeatureExtractionModeVal.getText().toString()) == 2){
                    HybridArt2FileName = folderName + "HybridArt2Mel_clusterParameters" + ".dat";}
                else if (Integer.parseInt(FeatureExtractionModeVal.getText().toString()) == 3){
                    HybridArt2FileName = folderName + "HybridArt2MelSubband_clusterParameters" + ".dat";}*/

        }

        System.loadLibrary("SuperpoweredExample");
        FrequencyDomain(Integer.parseInt(samplerateString),
                Integer.parseInt(buffersizeString),
                IsNoiseDetected,
                HybridSVDDFileName,
                HybridArt2FileName,
                StoreFeaturesFileName,
                DigitDecisionSmoothBuff, //Integer.parseInt(DecisionSmoothingBuffVal.getText().toString()),
                DigitNewClusterCreaBuff, //Integer.parseInt(NewClusterCreaBuffVal.getText().toString()),
                Integer.parseInt(FeatAvgBuffVal.getText().toString()), //decisionBufferLength,
                HybridClassificationSwitch.isChecked(),
                SavingClassificationDataSwitch.isChecked(),
                storeFeaturesSwitch.isChecked(),
                Float.parseFloat(VigilanceVal1.getText().toString()),
                Float.parseFloat(VigilanceVal2.getText().toString()) );
        handler.postDelayed(r,CNNUpdate);
        handler.postDelayed(timeUpdate, 1000);

        buttonStart.setBackgroundColor(Color.LTGRAY);
        buttonStop.setBackgroundColor(Color.RED);



    }

    public void buttonStopClick(View view){
        StopAudio(StoreFeaturesFileName); // modified by Nasim
        enableButtons();
        //statusView.append("\n Recording Stopped\n");
        handler.removeCallbacks(r);
        handler.removeCallbacks(timeUpdate);
        buttonStart.setBackgroundColor(Color.GREEN);
        buttonStop.setBackgroundColor(Color.LTGRAY);
    }

    public void getTime(){

        statusView.setMovementMethod(new ScrollingMovementMethod());
        if (getClusterLabel() >= 0 & getClusterLabel() <20){
            statusView.append(
                    //"Frame Processing Time: " +
                    //new DecimalFormat("##.##").format(getExecutionTime()) +
                    //" ms\nDetected Class:" + getDetectedClassLabel() +
                    "Classified class: " + getClusterLabel() + " out of " + getTotalClusters() + " classes.\n"
                           // + " | dB Power: " + new DecimalFormat("##").format(getdbPower()) + " dBFS\n"
                    //+ "********************** \n"
            );
        }
        else {
            statusView.append(
                    //"Frame Processing Time: " +
                    //new DecimalFormat("##.##").format(getExecutionTime()) +
                    //" ms\nDetected Class:" + getDetectedClassLabel() +
                    "Classified class: " + 0 + " out of " + getTotalClusters() + " classes.\n"
                           //  + " | dB Power: " + new DecimalFormat("##").format(getdbPower()) + " dBFS\n"
                    //+ "********************** \n"
            );
        }
        final int scrollAmount = statusView.getLayout().getLineTop(statusView.getLineCount()) - statusView.getHeight();
        if (scrollAmount > 0)
            statusView.scrollTo(0, scrollAmount);
        else
            statusView.scrollTo(0, 0);
    }

    public void onReadClick(View view) {

        int bufferSize; //should be 600
        int StepSize = 25;
        //int decisionBufferLength = (int)((Float.parseFloat(DecisionRateVal.getText().toString()) * 1000)/(StepSize/2)); //for decision rate of 0.5 sec, should be 40
        //int FeatAvgBuffVal = 80;
        // if FeatAvgBuffVal = 80, OverlapSize = 12.5 ms, NewClusterCreaBuffVal = 3 sec, then DigitNewClusterCreaBuff = 3.
        int DigitNewClusterCreaBuff = (int)((Float.parseFloat(NewClusterCreaBuffVal.getText().toString()) * 1000)/( (StepSize/2) * Integer.parseInt(FeatAvgBuffVal.getText().toString())) ); //Integer.parseInt(FeatAvgBuffVal.getText().toString())
        int DigitDecisionSmoothBuff = (int)((Float.parseFloat(DecisionSmoothingBuffVal.getText().toString()) * 1000)/( (StepSize/2) * Integer.parseInt(FeatAvgBuffVal.getText().toString())) ); //Integer.parseInt(FeatAvgBuffVal.getText().toString())
        bufferSize = (int) ((48000 * StepSize)/(2*1000)); //getSamplingRate(), Float.parseFloat(OverlapSizeVal.getText().toString()

        //statusView.append("\nRecording Started\n");

        if(storeFeaturesSwitch.isChecked()){
            StoreFeaturesFileName = folderName + new SimpleDateFormat("yyyy_MM_dd_HH_mm_ss").format(new Date()) + ".txt";
            Log.d("Filename",StoreFeaturesFileName);
        }

        if(HybridClassificationSwitch.isChecked() || SavingClassificationDataSwitch.isChecked()) {

            HybridSVDDFileName = folderName + "HybridSVDD_clusterParameters" + ".dat";
            Log.d("Filename", HybridSVDDFileName);

            HybridArt2FileName = folderName + "HybridArt2Fusion_clusterParameters" + ".dat";

            /*if (Integer.parseInt(FeatureExtractionModeVal.getText().toString()) == 1){
                HybridArt2FileName = folderName + "HybridArt2Subband_clusterParameters" + ".dat";}
            else if (Integer.parseInt(FeatureExtractionModeVal.getText().toString()) == 2){
                HybridArt2FileName = folderName + "HybridArt2Mel_clusterParameters" + ".dat";}
            else if (Integer.parseInt(FeatureExtractionModeVal.getText().toString()) == 3){
                HybridArt2FileName = folderName + "HybridArt2MelSubband_clusterParameters" + ".dat";}*/

        }

        System.loadLibrary("SuperpoweredExample");
        String path = folderName;
        Log.d("Files", "Path: " + path);
        File directory = new File(path);
        final File[] files = directory.listFiles();
        Log.d("Files", "Size: "+ files.length);
        List<String> filenames = new ArrayList<String>();
        for (int i = 0; i < files.length; i++)
        {
            Log.d("Files", "FileName: " + getExtensionOfFile(files[i].getName()));

            if (getExtensionOfFile(files[i].getName()).equals("wav")){
                filenames.add(files[i].getName());
            }

        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Choose Audio File:");
        final CharSequence[] charSequenceItems = filenames.toArray(new CharSequence[filenames.size()]);
        builder.setItems(charSequenceItems, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                Log.d("Files", String.valueOf(which));
                audioFileName = folderName + charSequenceItems[which];
                System.loadLibrary("SuperpoweredExample");
                ReadFile(Integer.parseInt(samplerateString),
                        Integer.parseInt(buffersizeString),
                        IsNoiseDetected,
                        HybridSVDDFileName,
                        HybridArt2FileName,
                        StoreFeaturesFileName,
                        DigitDecisionSmoothBuff, //Integer.parseInt(DecisionSmoothingBuffVal.getText().toString()),
                        DigitNewClusterCreaBuff, //Integer.parseInt(NewClusterCreaBuffVal.getText().toString()),
                        Integer.parseInt(FeatAvgBuffVal.getText().toString()),  //decisionBufferLength,
                        HybridClassificationSwitch.isChecked(),
                        SavingClassificationDataSwitch.isChecked(),
                        storeFeaturesSwitch.isChecked(),
                        Float.parseFloat(VigilanceVal1.getText().toString()),
                        Float.parseFloat(VigilanceVal2.getText().toString()),
                        audioFileName); //playAudioSwitch.isChecked())
                disableButtons();
                handler.postDelayed(r,CNNUpdate);
                handler.postDelayed(timeUpdate, 1000);

                buttonStart.setBackgroundColor(Color.LTGRAY);
                buttonStop.setBackgroundColor(Color.RED);
            }
        });
        builder.show();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Get the device's sample rate and buffer size to enable low-latency Android audio output, if available.

        if (Build.VERSION.SDK_INT >= 17) {
            AudioManager audioManager = (AudioManager) this.getSystemService(Context.AUDIO_SERVICE);
            samplerateString = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            buffersizeString = "600";//audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        }
        if (samplerateString == null) samplerateString = "44100";
        if (buffersizeString == null) buffersizeString = "512";


        initializeVariables(Integer.parseInt(samplerateString), Integer.parseInt(buffersizeString));
        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        prefEdit = prefs.edit();
        loadUserDefaults();
        enableButtons();
        File folder = new File(folderName);
        if(!folder.exists()){
            folder.mkdirs();
        }

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
    public static String getExtensionOfFile(String name)
    {
        String fileExtension="";

        // If fileName do not contain "." or starts with "." then it is not a valid file
        if(name.contains(".") && name.lastIndexOf(".")!= 0)
        {
            fileExtension=name.substring(name.lastIndexOf(".")+1);
        }

        return fileExtension;
    }

    private native void FrequencyDomain(int samplerate,
                                        int buffersize,
                                        int IsNoiseDetected,
                                        String HybridSVDDFileName,
                                        String HybridArt2FileName,
                                        String StoreFeaturesFileName,
                                        float decisionSmoothingBuffer,
                                        int NewclusterCreationBuffer,
                                        int FeatAvgBufferLength,
                                        boolean HybridButton,
                                        boolean SavingClassifDataButton,
                                        boolean storeFeaturesButton,
                                        float VigilanceVal1,
                                        float VigilanceVal2);
    private native void ReadFile(int samplerate,
                                 int buffersize,
                                 int IsNoiseDetected,
                                 String HybridSVDDFileName,
                                 String HybridArt2FileName,
                                 String StoreFeaturesFileName,
                                 float decisionSmoothingBuffer,
                                 int NewclusterCreationBuffer,
                                 int FeatAvgBufferLength,
                                 boolean HybridButton,
                                 boolean SavingClassifDataButton,
                                 boolean storeFeaturesButton,
                                 float VigilanceVal1,
                                 float VigilanceVal2,
                                 String audioFileName); //boolean playAudio
    private native float[] GetMelImage();
    private native float getProcessingTime();
    private native void StopAudio(String storeFeaturesFileName);
    private native int getClusterLabel ();
    private native int getTotalClusters();
    private native float getdbPower();
}


