package ssdb;

import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;

public final class Jni
{
    public static final int WRITERS = Integer.getInteger("ssdb.writers", 1);
    public static final int READERS = Integer.getInteger("ssdb.readers", 2);
    
    public static final byte[][] BUF_WRITERS = new byte[WRITERS][0xFFFF];
    public static final byte[][] BUF_READERS = new byte[READERS][0xFFFF];
    
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
                if (!initThread(ptrFn, ptrArg, get(type)[id]))
                    System.err.println("Could not start thread " + id + " of type: " + type);
            }
        }).start();
    }
    
    static final byte[] RESPONSE = new byte[]{
        '[', '0', ']'
    };
    
    public static void handle(int type, int id)
    {
        final byte[] buffer = get(type)[id];
        int offset = 0,
                headerSize = ((buffer[offset++] & 0xFF) | (buffer[offset++] & 0xFF) << 8),
                payloadSize = ((buffer[offset++] & 0xFF) | (buffer[offset++] & 0xFF) << 8);
        
        //System.err.println("h: " + headerSize + " | p: " + payloadSize);
        
        int tip = offset + headerSize + payloadSize,
                responseSize = RESPONSE.length;
        
        buffer[tip++] = (byte)(responseSize & 0xFF);
        buffer[tip++] = (byte)((responseSize >>>  8) & 0xFF);
        
        System.arraycopy(RESPONSE, 0, buffer, tip, responseSize);
    }
    
    public static void main(String[] args)
    {
        for (MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans())
        {
            System.out.println(memoryPoolMXBean.getName());
            System.out.println(memoryPoolMXBean.getUsage().getUsed());
        }
        
        System.out.println("ssdb.Jni main - w: " + WRITERS + ", r: " + READERS);
        init(WRITERS, READERS);
    }
  
}
