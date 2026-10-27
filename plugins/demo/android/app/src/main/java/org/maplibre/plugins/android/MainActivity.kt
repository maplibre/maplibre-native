package org.maplibre.plugins.android

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.ListView

/** Lists the demo of every plugin. `am start -n .../.DemoActivity -e plugin <id>` opens one directly. */
class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val demos = Demo.all(this)
        val list = ListView(this)
        list.fitsSystemWindows = true
        list.adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, demos.map { it.displayName })
        list.setOnItemClickListener { _, _, position, _ ->
            startActivity(
                Intent(this, DemoActivity::class.java)
                    .putExtra("plugin", demos[position].id)
                    .putExtra("renderer", intent.getStringExtra("renderer"))
            )
        }
        setContentView(list)
    }
}
