package com.mcntech.udpplayer;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class VideoFeedPosDb extends SQLiteOpenHelper {

	private static final int DATABASE_VERSION = 1;
	protected static final String DATABASE_NAME = "VrRenderDb";
	protected static final String TABLE_FEED_POSITION = "VideoFeeds";

    public static class FeedPos
    {
		public static final String URL = "url";
		public static final String POSITION = "position";
    	
    	public FeedPos()
    	{
    	}
    }


    public VideoFeedPosDb(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }
    
	@Override
	public void onCreate(SQLiteDatabase db) {
		// TODO Auto-generated method stub
        String sql = "CREATE TABLE " + TABLE_FEED_POSITION +
                "( id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                FeedPos.URL + " TEXT, " +
                FeedPos.POSITION + " TEXT ) ";
 
        db.execSQL(sql);		
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
		 
        String sql = "DROP TABLE IF EXISTS " + TABLE_FEED_POSITION;
        db.execSQL(sql);
        onCreate(db);
	}
	
    public int getPos(String url) {
        
        SQLiteDatabase db = this.getWritableDatabase();
        Cursor cursor=null;
        int position = 0;
        String p_query = "SELECT * FROM " + TABLE_FEED_POSITION + " WHERE " + FeedPos.URL + " = " + "?";
        cursor = db.rawQuery(p_query, new String[] { url });
        if (cursor != null)  {
             if (cursor.moveToFirst()){
            	 position = cursor.getInt(cursor.getColumnIndex(FeedPos.POSITION));
             }
             cursor.close();
        }
        db.close();
        return position;
    }

    public String getUrl(int  pos) {
        
        SQLiteDatabase db = this.getWritableDatabase();
        Cursor cursor=null;
        String url = null;
        String p_query = "SELECT * FROM " + TABLE_FEED_POSITION + " WHERE " + FeedPos.POSITION + " = " + pos;
        cursor = db.rawQuery(p_query, new String[] {});
        if (cursor != null)  {
             if (cursor.moveToFirst()){
            	 url = cursor.getString(cursor.getColumnIndex(FeedPos.URL));
             }
             cursor.close();
        }
        db.close();
        return url;
    }
    
    public  boolean insertFeed(String url, int pos) {
        SQLiteDatabase db = this.getWritableDatabase();  
        ContentValues values = new ContentValues();
        values.put(FeedPos.URL, url);
        values.put(FeedPos.POSITION, pos);
          
        boolean insertSuccessful = db.insert(TABLE_FEED_POSITION, null, values) > 0;
        db.close();
     
        return insertSuccessful;
    }
    
    public void clear()
    {
        SQLiteDatabase db = this.getWritableDatabase();  
        db.delete(TABLE_FEED_POSITION, null, null);
    }
    
    public  boolean setPos(String url, int pos) {
        SQLiteDatabase db = this.getWritableDatabase();  
        ContentValues values = new ContentValues();
        values.put(FeedPos.URL, url);
        values.put(FeedPos.POSITION, pos);

        String where = FeedPos.URL + " = ?";
        String[] whereArgs = { url };
          
        boolean updateSuccessful = db.update(TABLE_FEED_POSITION, values, where, whereArgs) > 0;
        db.close();
     
        return updateSuccessful;
    }
    public  long getNumFeeds() {
	    SQLiteDatabase db = getReadableDatabase();
	    return DatabaseUtils.queryNumEntries(db, TABLE_FEED_POSITION);
    }
}
