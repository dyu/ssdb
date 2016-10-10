package ssdb;

public final class Jni
{
    public static final boolean USE_UNSAFE_BAO = Boolean.getBoolean("jni.unsafe_bao");
    
    public static final sun.misc.Unsafe UNSAFE = initUnsafe();
    public static final int BYTE_ARRAY_OFFSET = UNSAFE.arrayBaseOffset(byte[].class);
    public static final int BAO = USE_UNSAFE_BAO ? BYTE_ARRAY_OFFSET : 0;

    private static sun.misc.Unsafe initUnsafe()
    {
        try
        {
            java.lang.reflect.Field f = sun.misc.Unsafe.class.getDeclaredField("theUnsafe");
            
            f.setAccessible(true);
            
            return (sun.misc.Unsafe)f.get(null);
        }
        catch (Exception e)
        {
            // ignore
        }
        
        return sun.misc.Unsafe.getUnsafe();
    }
    
    public static native boolean init(byte[] data, int bao);
    
    public static void main(String[] args)
    {
        init(null, BAO);
    }
  
}
