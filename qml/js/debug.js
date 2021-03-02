.pragma library
.import harbour.valuelogger 1.0 as ValueLogger

var enabled = ValueLogger.DebugLog.enabled

var log = enabled ? console.log : function(){};
