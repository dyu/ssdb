package ssdb;

public final class Jni
{
    public static final int WRITERS = Integer.getInteger("ssdb.writers", 1);
    public static final int READERS = Integer.getInteger("ssdb.readers", 2);
    
    public static final byte[][] BUF_WRITERS = new byte[WRITERS][];
    public static final byte[][] BUF_READERS = new byte[READERS][];
    
    public static final boolean USE_UNSAFE_BAO = Boolean.getBoolean("jni.unsafe_bao");
    
    public static final sun.misc.Unsafe UNSAFE = initUnsafe();
    public static final int BYTE_ARRAY_OFFSET = UNSAFE.arrayBaseOffset(byte[].class);
    public static final int BAO = USE_UNSAFE_BAO ? BYTE_ARRAY_OFFSET : 0;

    public static byte[][] get(int type)
    {
        return type == 0 ? BUF_WRITERS : BUF_READERS;
    }
    
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
    
    public static native boolean init(int writers, int readers);
    public static native boolean initThread(long ptrFn, long ptrArg, byte[] data);
    
    public static void createThread(final long ptrFn, final long ptrArg, 
            final int type, final int id)
    {
        new Thread(new Runnable()
        {
            @Override
            public void run()
            {
                final byte[] buf = new byte[0xFFFF];
                get(type)[id] = buf;
                if (!initThread(ptrFn, ptrArg, buf))
                    System.err.println("Could not start thread " + id + " of type: " + type);
                //get(type)[id] = null;
            }
        }).start();
    }
    
    public static void handle(int type, int id)
    {
        // TODO
    }
    
    public static void main(String[] args)
    {
        System.out.println("ssdb.Jni main - w: " + WRITERS + ", r: " + READERS);
        init(WRITERS, READERS);
    }
  
}
