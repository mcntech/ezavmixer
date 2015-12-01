package com.mcntech.ezscreencast;

import java.util.ArrayList;
import java.util.List;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class DatabaseHandler extends SQLiteOpenHelper {
	 
    private static final int DATABASE_VERSION = 1;
    protected static final String DATABASE_NAME = "RemoteNodesDatabase";
    
    protected static final String TABLE_NODES = "remotenodes";
    
    public DatabaseHandler(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }
 
    @Override
    public void onCreate(SQLiteDatabase db) {
 
        String sql = "CREATE TABLE " + TABLE_NODES +
                "( id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                OnyxRemoteNode.NODE_NICKNAME + " TEXT, " +
                OnyxRemoteNode.NODE_HOST + " TEXT, " +
                OnyxRemoteNode.NODE_ACCESSID + " TEXT, " +
                OnyxRemoteNode.NODE_SECURITYKEY + " TEXT, " +
                OnyxRemoteNode.NODE_FILEPREFIX + " TEXT, " +
                OnyxRemoteNode.NODE_FOLDER + " TEXT, " +
                OnyxRemoteNode.NODE_BUCKET + " TEXT ) ";
 
        db.execSQL(sql);
 
    }
 
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
 
        String sql = "DROP TABLE IF EXISTS " + TABLE_NODES;
        db.execSQL(sql);
 
        onCreate(db);
    }

    public boolean createRow(OnyxRemoteNode node){
    	ContentValues values = new ContentValues();
    	CopyNodeToTableRow(values, node);	 
	    SQLiteDatabase db = this.getWritableDatabase();
	 
	    boolean createSuccessful = db.insert(TABLE_NODES, null, values) > 0;
	    db.close();
	 
	    return createSuccessful;
    }
    
    public int count() {
    	 
        SQLiteDatabase db = this.getWritableDatabase();
     
        String sql = "SELECT * FROM " + TABLE_NODES;
        int recordCount = db.rawQuery(sql, null).getCount();
        db.close();
     
        return recordCount;
     
    }
    void CopyTableRowToNode(OnyxRemoteNode objectNode, Cursor cursor)
    {    
	    objectNode.mNickname = cursor.getString(cursor.getColumnIndex(OnyxRemoteNode.NODE_NICKNAME));
	    objectNode.mHost = cursor.getString(cursor.getColumnIndex(OnyxRemoteNode.NODE_HOST));
	    objectNode.mAccessid = cursor.getString(cursor.getColumnIndex(OnyxRemoteNode.NODE_ACCESSID));
	    objectNode.mSecuritykey = cursor.getString(cursor.getColumnIndex(OnyxRemoteNode.NODE_SECURITYKEY));
	    objectNode.mFileprefix = cursor.getString(cursor.getColumnIndex(OnyxRemoteNode.NODE_FILEPREFIX));
	    objectNode.mFolder = cursor.getString(cursor.getColumnIndex(OnyxRemoteNode.NODE_FOLDER));
	    objectNode.mBucket = cursor.getString(cursor.getColumnIndex(OnyxRemoteNode.NODE_BUCKET));
    }

    void CopyNodeToTableRow(ContentValues values, OnyxRemoteNode node)
    {  
	    values.put(OnyxRemoteNode.NODE_NICKNAME, node.mNickname);
	    values.put(OnyxRemoteNode.NODE_HOST, node.mHost);
	    values.put(OnyxRemoteNode.NODE_ACCESSID, node.mAccessid);
	    values.put(OnyxRemoteNode.NODE_SECURITYKEY, node.mSecuritykey);
	    values.put(OnyxRemoteNode.NODE_FILEPREFIX, node.mFileprefix);
	    values.put(OnyxRemoteNode.NODE_FOLDER, node.mFolder);
	    values.put(OnyxRemoteNode.NODE_BUCKET, node.mBucket);
    }

    public List<OnyxRemoteNode> read() {
    	 
        List<OnyxRemoteNode> recordsList = new ArrayList<OnyxRemoteNode>();
     
        String sql = "SELECT * FROM " + TABLE_NODES + " ORDER BY nickname DESC";
     
        SQLiteDatabase db = this.getWritableDatabase();
        Cursor cursor = db.rawQuery(sql, null);
     
        if (cursor.moveToFirst()) {
            do {
                OnyxRemoteNode objectNode = new OnyxRemoteNode();
                CopyTableRowToNode(objectNode, cursor);    
                recordsList.add(objectNode);
     
            } while (cursor.moveToNext());
        }
     
        cursor.close();
        db.close();
     
        return recordsList;
    }
    
    public OnyxRemoteNode read(String nickname) {
     
        SQLiteDatabase db = this.getWritableDatabase();
        Cursor cursor=null;
        OnyxRemoteNode objectNode = null;
        String p_query = "SELECT * FROM " + TABLE_NODES + " WHERE " + OnyxRemoteNode.NODE_NICKNAME + " = " + "?";
        cursor = db.rawQuery(p_query, new String[] { nickname });
        if (cursor != null)  {
             if (cursor.moveToFirst()){
            	 objectNode = new OnyxRemoteNode();
            	 CopyTableRowToNode(objectNode, cursor); 
             }
             cursor.close();
        }
        db.close();
        return objectNode;
    }
    
    public  boolean update(OnyxRemoteNode node) {
        SQLiteDatabase db = this.getWritableDatabase();  
        ContentValues values = new ContentValues();
        CopyNodeToTableRow(values, node);

        String where = OnyxRemoteNode.NODE_NICKNAME + " = ?";
        String[] whereArgs = { node.mNickname };
          
        boolean updateSuccessful = db.update(TABLE_NODES, values, where, whereArgs) > 0;
        db.close();
     
        return updateSuccessful;
    }

}
