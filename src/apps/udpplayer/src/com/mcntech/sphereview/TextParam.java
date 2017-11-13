package com.mcntech.sphereview;

public class TextParam {
	float blX;
	float blY;
	float tlX;
	float tlY;
	float trX;
	float trY;
	float brX;
	float brY;
	
	
	public static final String BL_X = "BL_X";
	public static final String BL_Y = "BL_Y";
	public static final String TL_X = "TL_X";
	public static final String TL_Y = "TL_Y";
	public static final String TR_X = "TR_X";
	public static final String TR_Y = "TR_Y";
	public static final String BR_X = "BR_X";
	public static final String BR_Y = "BR_Y";
	
	TextParam()
	{
		blX = (float)0.0;
		blY = (float)0.0;
		tlX = (float)0.0;
		tlY = (float)1.0;
		trX = (float)1.0;
		trY = (float)1.0;
		trX = (float)1.0;
		trY = (float)0.0;
	}
}
