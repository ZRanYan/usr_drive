
#ifndef _USR_DEBUG_H_
#define _USR_DEBUG_H_

#define FILE_NAME(str) strrchr(str, '/') ? strrchr(str, '/') + 1 : str

// #define USR_DEBUG_ENABLE

#ifdef USR_DEBUG_ENABLE
    #define vc_info(dev, fmt, ...) \
        dev_info(dev, "[%s:%d %s]:" fmt, FILE_NAME(__FILE__), __LINE__, __func__, ##__VA_ARGS__)
   #define vc_notice(dev, fmt, ...) \
        dev_notice(dev, "[%s:%d %s]:" fmt, FILE_NAME(__FILE__), __LINE__, __func__, ##__VA_ARGS__) //用作打印初始化sensor列表
    #define vc_warn(dev, fmt, ...) \
        dev_warn(dev, "[%s:%d %s]:" fmt, FILE_NAME(__FILE__), __LINE__, __func__, ##__VA_ARGS__)
    #define vc_err(dev, fmt, ...) \
        dev_err(dev, "[%s:%d %s]:" fmt, FILE_NAME(__FILE__), __LINE__, __func__, ##__VA_ARGS__)
#else
    #define vc_info(dev, fmt, ...)  
    #define vc_notice(dev, fmt, ...) 
    #define vc_warn(dev, fmt, ...)  
    #define vc_err(dev, fmt, ...)  
#endif
    
#endif


