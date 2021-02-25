.pragma library
.import harbour.valuelogger.Logger 1.0 as Logger

var enabled = Logger.DebugLog.enabled

var log = enabled ? console.log : function(){};
