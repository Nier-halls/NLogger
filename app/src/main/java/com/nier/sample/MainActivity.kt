package com.nier.sample

import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import android.util.Log
import android.widget.EditText
import android.widget.TextView
import com.nier.nlogger.NLogger
import com.nier.nlogger.R
import kotlinx.coroutines.*
import kotlin.coroutines.CoroutineContext

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        NLogger.init(this)

//        findViewById<EditText>(R.id.tv_result).setText("Hello World QWERTYUIOPASDFGHJKL[]{};':ZXCVBNM<>?,./123456789!@#\\\$%^*()!")
//
//        findViewById<TextView>(R.id.btn_init).setOnClickListener {
//            //            NLogger.init(this)
//            testMutiThread(50)
//        }
//
//        findViewById<TextView>(R.id.btn_write).setOnClickListener {
//            val text = findViewById<EditText>(R.id.tv_result).text
//            NLogger.write("date[${System.currentTimeMillis()}] content[$text]")
//
//        }
//
//        findViewById<TextView>(R.id.btn_flush).setOnClickListener {
//            NLogger.flush()
//        }


//        NLoggerProxy.instance.init(this)
//        NLoggerProxy.instance.write("")
//        NLoggerProxy.instance.write("")
//        NLoggerProxy.instance.write("")
//        NLoggerProxy.instance.write("")
//        NLoggerProxy.instance.flush()
        testCoroutine()
    }

    private fun testCoroutine() {
        val TAG = "fgd"
        Log.d(TAG, "Coroutine start")
        val test1 = runBlocking {
            coroutineScope {
                launch {
                    delay(3000)
                    Log.d(TAG, "Coroutine coroutineScope launch finish ## currentThread().name >>> ${Thread.currentThread().name}")
                }
                Log.d(TAG, "Coroutine coroutineScope ## currentThread().name >>> ${Thread.currentThread().name}")
            }

            Log.d(TAG, "Coroutine runBlocking finish")
        }
        Log.d(TAG, "testCoroutine finish")
    }


    private fun testMutiThread(threadNum: Int) {
        for (i in 0..threadNum) {
            Thread {
                val result = i % 3
                when (result) {
                    0 -> NLogger.write("Hello World QWERTYUIOPASDFGHJKL[]{};':ZXCVBNM<>?,./123456789!@#\\\$%^*()!")
                    1 -> NLogger.send()
                    2 -> NLogger.flush()
                }
            }.start()
        }
    }
}