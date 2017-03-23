using System;


public class rfPacket{
	public int Length;
	public int Type;
	public int[] dados;

	public rfPacket(){
		this.dados = new int[32];
	}
}
