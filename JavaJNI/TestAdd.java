public class TestAdd
{
	public static void main(String[] args)
	{
		System.loadLibrary("NativeAdd");
        NativeAdd na = new NativeAdd();
		System.out.println("3 + 4 = " + na.add(3, 4));
	}
}
class NativeAdd
{
    public native int add(int x, int y);
}

