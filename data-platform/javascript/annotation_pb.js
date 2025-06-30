// source: annotation.proto
/**
 * @fileoverview
 * @enhanceable
 * @suppress {missingRequire} reports error on implicit type usages.
 * @suppress {messageConventions} JS Compiler reports an error if a variable or
 *     field starts with 'MSG_' and isn't a translatable message.
 * @public
 */
// GENERATED CODE -- DO NOT EDIT!
/* eslint-disable */
// @ts-nocheck

var jspb = require('google-protobuf');
var goog = jspb;
var global =
    (typeof globalThis !== 'undefined' && globalThis) ||
    (typeof window !== 'undefined' && window) ||
    (typeof global !== 'undefined' && global) ||
    (typeof self !== 'undefined' && self) ||
    (function () { return this; }).call(null) ||
    Function('return this')();

var common_pb = require('./common_pb.js');
goog.object.extend(proto, common_pb);
goog.exportSymbol('proto.dp.service.annotation.Calculations', null, global);
goog.exportSymbol('proto.dp.service.annotation.Calculations.CalculationsDataFrame', null, global);
goog.exportSymbol('proto.dp.service.annotation.CreateAnnotationRequest', null, global);
goog.exportSymbol('proto.dp.service.annotation.CreateAnnotationResponse', null, global);
goog.exportSymbol('proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult', null, global);
goog.exportSymbol('proto.dp.service.annotation.CreateAnnotationResponse.ResultCase', null, global);
goog.exportSymbol('proto.dp.service.annotation.CreateDataSetRequest', null, global);
goog.exportSymbol('proto.dp.service.annotation.CreateDataSetResponse', null, global);
goog.exportSymbol('proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult', null, global);
goog.exportSymbol('proto.dp.service.annotation.CreateDataSetResponse.ResultCase', null, global);
goog.exportSymbol('proto.dp.service.annotation.DataBlock', null, global);
goog.exportSymbol('proto.dp.service.annotation.DataSet', null, global);
goog.exportSymbol('proto.dp.service.annotation.ExportDataRequest', null, global);
goog.exportSymbol('proto.dp.service.annotation.ExportDataRequest.ExportOutputFormat', null, global);
goog.exportSymbol('proto.dp.service.annotation.ExportDataResponse', null, global);
goog.exportSymbol('proto.dp.service.annotation.ExportDataResponse.ExportDataResult', null, global);
goog.exportSymbol('proto.dp.service.annotation.ExportDataResponse.ResultCase', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.CriterionCase', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsResponse', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryAnnotationsResponse.ResultCase', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsRequest', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.CriterionCase', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsResponse', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult', null, global);
goog.exportSymbol('proto.dp.service.annotation.QueryDataSetsResponse.ResultCase', null, global);
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.DataSet = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.DataSet.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.DataSet, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.DataSet.displayName = 'proto.dp.service.annotation.DataSet';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.DataBlock = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.DataBlock.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.DataBlock, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.DataBlock.displayName = 'proto.dp.service.annotation.DataBlock';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.CreateDataSetRequest = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.CreateDataSetRequest, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.CreateDataSetRequest.displayName = 'proto.dp.service.annotation.CreateDataSetRequest';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.CreateDataSetResponse = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, proto.dp.service.annotation.CreateDataSetResponse.oneofGroups_);
};
goog.inherits(proto.dp.service.annotation.CreateDataSetResponse, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.CreateDataSetResponse.displayName = 'proto.dp.service.annotation.CreateDataSetResponse';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.displayName = 'proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryDataSetsRequest = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.QueryDataSetsRequest.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.QueryDataSetsRequest, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryDataSetsRequest.displayName = 'proto.dp.service.annotation.QueryDataSetsRequest';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.oneofGroups_);
};
goog.inherits(proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.displayName = 'proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.displayName = 'proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.displayName = 'proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.displayName = 'proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.displayName = 'proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryDataSetsResponse = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, proto.dp.service.annotation.QueryDataSetsResponse.oneofGroups_);
};
goog.inherits(proto.dp.service.annotation.QueryDataSetsResponse, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryDataSetsResponse.displayName = 'proto.dp.service.annotation.QueryDataSetsResponse';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.displayName = 'proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.Calculations = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.Calculations.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.Calculations, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.Calculations.displayName = 'proto.dp.service.annotation.Calculations';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.Calculations.CalculationsDataFrame.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.Calculations.CalculationsDataFrame, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.Calculations.CalculationsDataFrame.displayName = 'proto.dp.service.annotation.Calculations.CalculationsDataFrame';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.CreateAnnotationRequest = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.CreateAnnotationRequest.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.CreateAnnotationRequest, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.CreateAnnotationRequest.displayName = 'proto.dp.service.annotation.CreateAnnotationRequest';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.CreateAnnotationResponse = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, proto.dp.service.annotation.CreateAnnotationResponse.oneofGroups_);
};
goog.inherits(proto.dp.service.annotation.CreateAnnotationResponse, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.CreateAnnotationResponse.displayName = 'proto.dp.service.annotation.CreateAnnotationResponse';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.displayName = 'proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.QueryAnnotationsRequest.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.displayName = 'proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsResponse = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, proto.dp.service.annotation.QueryAnnotationsResponse.oneofGroups_);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsResponse, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsResponse.displayName = 'proto.dp.service.annotation.QueryAnnotationsResponse';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.displayName = 'proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.repeatedFields_, null);
};
goog.inherits(proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.displayName = 'proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.ExportDataRequest = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.ExportDataRequest, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.ExportDataRequest.displayName = 'proto.dp.service.annotation.ExportDataRequest';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.ExportDataResponse = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, proto.dp.service.annotation.ExportDataResponse.oneofGroups_);
};
goog.inherits(proto.dp.service.annotation.ExportDataResponse, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.ExportDataResponse.displayName = 'proto.dp.service.annotation.ExportDataResponse';
}
/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.dp.service.annotation.ExportDataResponse.ExportDataResult, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  /**
   * @public
   * @override
   */
  proto.dp.service.annotation.ExportDataResponse.ExportDataResult.displayName = 'proto.dp.service.annotation.ExportDataResponse.ExportDataResult';
}

/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.DataSet.repeatedFields_ = [5];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.DataSet.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.DataSet.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.DataSet} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.DataSet.toObject = function(includeInstance, msg) {
  var f, obj = {
    id: jspb.Message.getFieldWithDefault(msg, 1, ""),
    name: jspb.Message.getFieldWithDefault(msg, 2, ""),
    ownerid: jspb.Message.getFieldWithDefault(msg, 3, ""),
    description: jspb.Message.getFieldWithDefault(msg, 4, ""),
    datablocksList: jspb.Message.toObjectList(msg.getDatablocksList(),
    proto.dp.service.annotation.DataBlock.toObject, includeInstance)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.DataSet}
 */
proto.dp.service.annotation.DataSet.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.DataSet;
  return proto.dp.service.annotation.DataSet.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.DataSet} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.DataSet}
 */
proto.dp.service.annotation.DataSet.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setId(value);
      break;
    case 2:
      var value = /** @type {string} */ (reader.readString());
      msg.setName(value);
      break;
    case 3:
      var value = /** @type {string} */ (reader.readString());
      msg.setOwnerid(value);
      break;
    case 4:
      var value = /** @type {string} */ (reader.readString());
      msg.setDescription(value);
      break;
    case 5:
      var value = new proto.dp.service.annotation.DataBlock;
      reader.readMessage(value,proto.dp.service.annotation.DataBlock.deserializeBinaryFromReader);
      msg.addDatablocks(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.DataSet.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.DataSet.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.DataSet} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.DataSet.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getId();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getName();
  if (f.length > 0) {
    writer.writeString(
      2,
      f
    );
  }
  f = message.getOwnerid();
  if (f.length > 0) {
    writer.writeString(
      3,
      f
    );
  }
  f = message.getDescription();
  if (f.length > 0) {
    writer.writeString(
      4,
      f
    );
  }
  f = message.getDatablocksList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      5,
      f,
      proto.dp.service.annotation.DataBlock.serializeBinaryToWriter
    );
  }
};


/**
 * optional string id = 1;
 * @return {string}
 */
proto.dp.service.annotation.DataSet.prototype.getId = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.DataSet} returns this
 */
proto.dp.service.annotation.DataSet.prototype.setId = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional string name = 2;
 * @return {string}
 */
proto.dp.service.annotation.DataSet.prototype.getName = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 2, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.DataSet} returns this
 */
proto.dp.service.annotation.DataSet.prototype.setName = function(value) {
  return jspb.Message.setProto3StringField(this, 2, value);
};


/**
 * optional string ownerId = 3;
 * @return {string}
 */
proto.dp.service.annotation.DataSet.prototype.getOwnerid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 3, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.DataSet} returns this
 */
proto.dp.service.annotation.DataSet.prototype.setOwnerid = function(value) {
  return jspb.Message.setProto3StringField(this, 3, value);
};


/**
 * optional string description = 4;
 * @return {string}
 */
proto.dp.service.annotation.DataSet.prototype.getDescription = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 4, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.DataSet} returns this
 */
proto.dp.service.annotation.DataSet.prototype.setDescription = function(value) {
  return jspb.Message.setProto3StringField(this, 4, value);
};


/**
 * repeated DataBlock dataBlocks = 5;
 * @return {!Array<!proto.dp.service.annotation.DataBlock>}
 */
proto.dp.service.annotation.DataSet.prototype.getDatablocksList = function() {
  return /** @type{!Array<!proto.dp.service.annotation.DataBlock>} */ (
    jspb.Message.getRepeatedWrapperField(this, proto.dp.service.annotation.DataBlock, 5));
};


/**
 * @param {!Array<!proto.dp.service.annotation.DataBlock>} value
 * @return {!proto.dp.service.annotation.DataSet} returns this
*/
proto.dp.service.annotation.DataSet.prototype.setDatablocksList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 5, value);
};


/**
 * @param {!proto.dp.service.annotation.DataBlock=} opt_value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.DataBlock}
 */
proto.dp.service.annotation.DataSet.prototype.addDatablocks = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 5, opt_value, proto.dp.service.annotation.DataBlock, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.DataSet} returns this
 */
proto.dp.service.annotation.DataSet.prototype.clearDatablocksList = function() {
  return this.setDatablocksList([]);
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.DataBlock.repeatedFields_ = [3];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.DataBlock.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.DataBlock.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.DataBlock} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.DataBlock.toObject = function(includeInstance, msg) {
  var f, obj = {
    begintime: (f = msg.getBegintime()) && common_pb.Timestamp.toObject(includeInstance, f),
    endtime: (f = msg.getEndtime()) && common_pb.Timestamp.toObject(includeInstance, f),
    pvnamesList: (f = jspb.Message.getRepeatedField(msg, 3)) == null ? undefined : f
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.DataBlock}
 */
proto.dp.service.annotation.DataBlock.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.DataBlock;
  return proto.dp.service.annotation.DataBlock.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.DataBlock} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.DataBlock}
 */
proto.dp.service.annotation.DataBlock.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new common_pb.Timestamp;
      reader.readMessage(value,common_pb.Timestamp.deserializeBinaryFromReader);
      msg.setBegintime(value);
      break;
    case 2:
      var value = new common_pb.Timestamp;
      reader.readMessage(value,common_pb.Timestamp.deserializeBinaryFromReader);
      msg.setEndtime(value);
      break;
    case 3:
      var value = /** @type {string} */ (reader.readString());
      msg.addPvnames(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.DataBlock.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.DataBlock.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.DataBlock} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.DataBlock.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getBegintime();
  if (f != null) {
    writer.writeMessage(
      1,
      f,
      common_pb.Timestamp.serializeBinaryToWriter
    );
  }
  f = message.getEndtime();
  if (f != null) {
    writer.writeMessage(
      2,
      f,
      common_pb.Timestamp.serializeBinaryToWriter
    );
  }
  f = message.getPvnamesList();
  if (f.length > 0) {
    writer.writeRepeatedString(
      3,
      f
    );
  }
};


/**
 * optional Timestamp beginTime = 1;
 * @return {?proto.Timestamp}
 */
proto.dp.service.annotation.DataBlock.prototype.getBegintime = function() {
  return /** @type{?proto.Timestamp} */ (
    jspb.Message.getWrapperField(this, common_pb.Timestamp, 1));
};


/**
 * @param {?proto.Timestamp|undefined} value
 * @return {!proto.dp.service.annotation.DataBlock} returns this
*/
proto.dp.service.annotation.DataBlock.prototype.setBegintime = function(value) {
  return jspb.Message.setWrapperField(this, 1, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.DataBlock} returns this
 */
proto.dp.service.annotation.DataBlock.prototype.clearBegintime = function() {
  return this.setBegintime(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.DataBlock.prototype.hasBegintime = function() {
  return jspb.Message.getField(this, 1) != null;
};


/**
 * optional Timestamp endTime = 2;
 * @return {?proto.Timestamp}
 */
proto.dp.service.annotation.DataBlock.prototype.getEndtime = function() {
  return /** @type{?proto.Timestamp} */ (
    jspb.Message.getWrapperField(this, common_pb.Timestamp, 2));
};


/**
 * @param {?proto.Timestamp|undefined} value
 * @return {!proto.dp.service.annotation.DataBlock} returns this
*/
proto.dp.service.annotation.DataBlock.prototype.setEndtime = function(value) {
  return jspb.Message.setWrapperField(this, 2, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.DataBlock} returns this
 */
proto.dp.service.annotation.DataBlock.prototype.clearEndtime = function() {
  return this.setEndtime(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.DataBlock.prototype.hasEndtime = function() {
  return jspb.Message.getField(this, 2) != null;
};


/**
 * repeated string pvNames = 3;
 * @return {!Array<string>}
 */
proto.dp.service.annotation.DataBlock.prototype.getPvnamesList = function() {
  return /** @type {!Array<string>} */ (jspb.Message.getRepeatedField(this, 3));
};


/**
 * @param {!Array<string>} value
 * @return {!proto.dp.service.annotation.DataBlock} returns this
 */
proto.dp.service.annotation.DataBlock.prototype.setPvnamesList = function(value) {
  return jspb.Message.setField(this, 3, value || []);
};


/**
 * @param {string} value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.DataBlock} returns this
 */
proto.dp.service.annotation.DataBlock.prototype.addPvnames = function(value, opt_index) {
  return jspb.Message.addToRepeatedField(this, 3, value, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.DataBlock} returns this
 */
proto.dp.service.annotation.DataBlock.prototype.clearPvnamesList = function() {
  return this.setPvnamesList([]);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.CreateDataSetRequest.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.CreateDataSetRequest.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.CreateDataSetRequest} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateDataSetRequest.toObject = function(includeInstance, msg) {
  var f, obj = {
    dataset: (f = msg.getDataset()) && proto.dp.service.annotation.DataSet.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.CreateDataSetRequest}
 */
proto.dp.service.annotation.CreateDataSetRequest.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.CreateDataSetRequest;
  return proto.dp.service.annotation.CreateDataSetRequest.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.CreateDataSetRequest} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.CreateDataSetRequest}
 */
proto.dp.service.annotation.CreateDataSetRequest.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new proto.dp.service.annotation.DataSet;
      reader.readMessage(value,proto.dp.service.annotation.DataSet.deserializeBinaryFromReader);
      msg.setDataset(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.CreateDataSetRequest.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.CreateDataSetRequest.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.CreateDataSetRequest} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateDataSetRequest.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getDataset();
  if (f != null) {
    writer.writeMessage(
      1,
      f,
      proto.dp.service.annotation.DataSet.serializeBinaryToWriter
    );
  }
};


/**
 * optional DataSet dataSet = 1;
 * @return {?proto.dp.service.annotation.DataSet}
 */
proto.dp.service.annotation.CreateDataSetRequest.prototype.getDataset = function() {
  return /** @type{?proto.dp.service.annotation.DataSet} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.DataSet, 1));
};


/**
 * @param {?proto.dp.service.annotation.DataSet|undefined} value
 * @return {!proto.dp.service.annotation.CreateDataSetRequest} returns this
*/
proto.dp.service.annotation.CreateDataSetRequest.prototype.setDataset = function(value) {
  return jspb.Message.setWrapperField(this, 1, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateDataSetRequest} returns this
 */
proto.dp.service.annotation.CreateDataSetRequest.prototype.clearDataset = function() {
  return this.setDataset(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateDataSetRequest.prototype.hasDataset = function() {
  return jspb.Message.getField(this, 1) != null;
};



/**
 * Oneof group definitions for this message. Each group defines the field
 * numbers belonging to that group. When of these fields' value is set, all
 * other fields in the group are cleared. During deserialization, if multiple
 * fields are encountered for a group, only the last value seen will be kept.
 * @private {!Array<!Array<number>>}
 * @const
 */
proto.dp.service.annotation.CreateDataSetResponse.oneofGroups_ = [[10,11]];

/**
 * @enum {number}
 */
proto.dp.service.annotation.CreateDataSetResponse.ResultCase = {
  RESULT_NOT_SET: 0,
  EXCEPTIONALRESULT: 10,
  CREATEDATASETRESULT: 11
};

/**
 * @return {proto.dp.service.annotation.CreateDataSetResponse.ResultCase}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.getResultCase = function() {
  return /** @type {proto.dp.service.annotation.CreateDataSetResponse.ResultCase} */(jspb.Message.computeOneofCase(this, proto.dp.service.annotation.CreateDataSetResponse.oneofGroups_[0]));
};



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.CreateDataSetResponse.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.CreateDataSetResponse} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateDataSetResponse.toObject = function(includeInstance, msg) {
  var f, obj = {
    responsetime: (f = msg.getResponsetime()) && common_pb.Timestamp.toObject(includeInstance, f),
    exceptionalresult: (f = msg.getExceptionalresult()) && common_pb.ExceptionalResult.toObject(includeInstance, f),
    createdatasetresult: (f = msg.getCreatedatasetresult()) && proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.CreateDataSetResponse}
 */
proto.dp.service.annotation.CreateDataSetResponse.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.CreateDataSetResponse;
  return proto.dp.service.annotation.CreateDataSetResponse.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.CreateDataSetResponse} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.CreateDataSetResponse}
 */
proto.dp.service.annotation.CreateDataSetResponse.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new common_pb.Timestamp;
      reader.readMessage(value,common_pb.Timestamp.deserializeBinaryFromReader);
      msg.setResponsetime(value);
      break;
    case 10:
      var value = new common_pb.ExceptionalResult;
      reader.readMessage(value,common_pb.ExceptionalResult.deserializeBinaryFromReader);
      msg.setExceptionalresult(value);
      break;
    case 11:
      var value = new proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult;
      reader.readMessage(value,proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.deserializeBinaryFromReader);
      msg.setCreatedatasetresult(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.CreateDataSetResponse.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.CreateDataSetResponse} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateDataSetResponse.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getResponsetime();
  if (f != null) {
    writer.writeMessage(
      1,
      f,
      common_pb.Timestamp.serializeBinaryToWriter
    );
  }
  f = message.getExceptionalresult();
  if (f != null) {
    writer.writeMessage(
      10,
      f,
      common_pb.ExceptionalResult.serializeBinaryToWriter
    );
  }
  f = message.getCreatedatasetresult();
  if (f != null) {
    writer.writeMessage(
      11,
      f,
      proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.serializeBinaryToWriter
    );
  }
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.toObject = function(includeInstance, msg) {
  var f, obj = {
    datasetid: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult}
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult;
  return proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult}
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setDatasetid(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getDatasetid();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string dataSetId = 1;
 * @return {string}
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.prototype.getDatasetid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult} returns this
 */
proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult.prototype.setDatasetid = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional Timestamp responseTime = 1;
 * @return {?proto.Timestamp}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.getResponsetime = function() {
  return /** @type{?proto.Timestamp} */ (
    jspb.Message.getWrapperField(this, common_pb.Timestamp, 1));
};


/**
 * @param {?proto.Timestamp|undefined} value
 * @return {!proto.dp.service.annotation.CreateDataSetResponse} returns this
*/
proto.dp.service.annotation.CreateDataSetResponse.prototype.setResponsetime = function(value) {
  return jspb.Message.setWrapperField(this, 1, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateDataSetResponse} returns this
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.clearResponsetime = function() {
  return this.setResponsetime(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.hasResponsetime = function() {
  return jspb.Message.getField(this, 1) != null;
};


/**
 * optional ExceptionalResult exceptionalResult = 10;
 * @return {?proto.ExceptionalResult}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.getExceptionalresult = function() {
  return /** @type{?proto.ExceptionalResult} */ (
    jspb.Message.getWrapperField(this, common_pb.ExceptionalResult, 10));
};


/**
 * @param {?proto.ExceptionalResult|undefined} value
 * @return {!proto.dp.service.annotation.CreateDataSetResponse} returns this
*/
proto.dp.service.annotation.CreateDataSetResponse.prototype.setExceptionalresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 10, proto.dp.service.annotation.CreateDataSetResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateDataSetResponse} returns this
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.clearExceptionalresult = function() {
  return this.setExceptionalresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.hasExceptionalresult = function() {
  return jspb.Message.getField(this, 10) != null;
};


/**
 * optional CreateDataSetResult createDataSetResult = 11;
 * @return {?proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.getCreatedatasetresult = function() {
  return /** @type{?proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult, 11));
};


/**
 * @param {?proto.dp.service.annotation.CreateDataSetResponse.CreateDataSetResult|undefined} value
 * @return {!proto.dp.service.annotation.CreateDataSetResponse} returns this
*/
proto.dp.service.annotation.CreateDataSetResponse.prototype.setCreatedatasetresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 11, proto.dp.service.annotation.CreateDataSetResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateDataSetResponse} returns this
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.clearCreatedatasetresult = function() {
  return this.setCreatedatasetresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateDataSetResponse.prototype.hasCreatedatasetresult = function() {
  return jspb.Message.getField(this, 11) != null;
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.QueryDataSetsRequest.repeatedFields_ = [1];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryDataSetsRequest.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryDataSetsRequest.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.toObject = function(includeInstance, msg) {
  var f, obj = {
    criteriaList: jspb.Message.toObjectList(msg.getCriteriaList(),
    proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.toObject, includeInstance)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest}
 */
proto.dp.service.annotation.QueryDataSetsRequest.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryDataSetsRequest;
  return proto.dp.service.annotation.QueryDataSetsRequest.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest}
 */
proto.dp.service.annotation.QueryDataSetsRequest.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.deserializeBinaryFromReader);
      msg.addCriteria(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryDataSetsRequest.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryDataSetsRequest.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getCriteriaList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      1,
      f,
      proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.serializeBinaryToWriter
    );
  }
};



/**
 * Oneof group definitions for this message. Each group defines the field
 * numbers belonging to that group. When of these fields' value is set, all
 * other fields in the group are cleared. During deserialization, if multiple
 * fields are encountered for a group, only the last value seen will be kept.
 * @private {!Array<!Array<number>>}
 * @const
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.oneofGroups_ = [[10,11,12,13]];

/**
 * @enum {number}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.CriterionCase = {
  CRITERION_NOT_SET: 0,
  IDCRITERION: 10,
  OWNERCRITERION: 11,
  TEXTCRITERION: 12,
  PVNAMECRITERION: 13
};

/**
 * @return {proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.CriterionCase}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.getCriterionCase = function() {
  return /** @type {proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.CriterionCase} */(jspb.Message.computeOneofCase(this, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.oneofGroups_[0]));
};



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    idcriterion: (f = msg.getIdcriterion()) && proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.toObject(includeInstance, f),
    ownercriterion: (f = msg.getOwnercriterion()) && proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.toObject(includeInstance, f),
    textcriterion: (f = msg.getTextcriterion()) && proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.toObject(includeInstance, f),
    pvnamecriterion: (f = msg.getPvnamecriterion()) && proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion;
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 10:
      var value = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.deserializeBinaryFromReader);
      msg.setIdcriterion(value);
      break;
    case 11:
      var value = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.deserializeBinaryFromReader);
      msg.setOwnercriterion(value);
      break;
    case 12:
      var value = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.deserializeBinaryFromReader);
      msg.setTextcriterion(value);
      break;
    case 13:
      var value = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.deserializeBinaryFromReader);
      msg.setPvnamecriterion(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getIdcriterion();
  if (f != null) {
    writer.writeMessage(
      10,
      f,
      proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.serializeBinaryToWriter
    );
  }
  f = message.getOwnercriterion();
  if (f != null) {
    writer.writeMessage(
      11,
      f,
      proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.serializeBinaryToWriter
    );
  }
  f = message.getTextcriterion();
  if (f != null) {
    writer.writeMessage(
      12,
      f,
      proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.serializeBinaryToWriter
    );
  }
  f = message.getPvnamecriterion();
  if (f != null) {
    writer.writeMessage(
      13,
      f,
      proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.serializeBinaryToWriter
    );
  }
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    id: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion;
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setId(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getId();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string id = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.prototype.getId = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion.prototype.setId = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    ownerid: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion;
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setOwnerid(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getOwnerid();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string ownerId = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.prototype.getOwnerid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion.prototype.setOwnerid = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    text: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion;
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setText(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getText();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string text = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.prototype.getText = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion.prototype.setText = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    name: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion;
  return proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setName(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getName();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string name = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.prototype.getName = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion.prototype.setName = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional IdCriterion idCriterion = 10;
 * @return {?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.getIdcriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion, 10));
};


/**
 * @param {?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.IdCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} returns this
*/
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.setIdcriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 10, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.clearIdcriterion = function() {
  return this.setIdcriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.hasIdcriterion = function() {
  return jspb.Message.getField(this, 10) != null;
};


/**
 * optional OwnerCriterion ownerCriterion = 11;
 * @return {?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.getOwnercriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion, 11));
};


/**
 * @param {?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.OwnerCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} returns this
*/
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.setOwnercriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 11, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.clearOwnercriterion = function() {
  return this.setOwnercriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.hasOwnercriterion = function() {
  return jspb.Message.getField(this, 11) != null;
};


/**
 * optional TextCriterion textCriterion = 12;
 * @return {?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.getTextcriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion, 12));
};


/**
 * @param {?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.TextCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} returns this
*/
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.setTextcriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 12, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.clearTextcriterion = function() {
  return this.setTextcriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.hasTextcriterion = function() {
  return jspb.Message.getField(this, 12) != null;
};


/**
 * optional PvNameCriterion pvNameCriterion = 13;
 * @return {?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.getPvnamecriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion, 13));
};


/**
 * @param {?proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.PvNameCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} returns this
*/
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.setPvnamecriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 13, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.clearPvnamecriterion = function() {
  return this.setPvnamecriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion.prototype.hasPvnamecriterion = function() {
  return jspb.Message.getField(this, 13) != null;
};


/**
 * repeated QueryDataSetsCriterion criteria = 1;
 * @return {!Array<!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion>}
 */
proto.dp.service.annotation.QueryDataSetsRequest.prototype.getCriteriaList = function() {
  return /** @type{!Array<!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion>} */ (
    jspb.Message.getRepeatedWrapperField(this, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion, 1));
};


/**
 * @param {!Array<!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion>} value
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest} returns this
*/
proto.dp.service.annotation.QueryDataSetsRequest.prototype.setCriteriaList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 1, value);
};


/**
 * @param {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion=} opt_value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion}
 */
proto.dp.service.annotation.QueryDataSetsRequest.prototype.addCriteria = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 1, opt_value, proto.dp.service.annotation.QueryDataSetsRequest.QueryDataSetsCriterion, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryDataSetsRequest} returns this
 */
proto.dp.service.annotation.QueryDataSetsRequest.prototype.clearCriteriaList = function() {
  return this.setCriteriaList([]);
};



/**
 * Oneof group definitions for this message. Each group defines the field
 * numbers belonging to that group. When of these fields' value is set, all
 * other fields in the group are cleared. During deserialization, if multiple
 * fields are encountered for a group, only the last value seen will be kept.
 * @private {!Array<!Array<number>>}
 * @const
 */
proto.dp.service.annotation.QueryDataSetsResponse.oneofGroups_ = [[10,11]];

/**
 * @enum {number}
 */
proto.dp.service.annotation.QueryDataSetsResponse.ResultCase = {
  RESULT_NOT_SET: 0,
  EXCEPTIONALRESULT: 10,
  DATASETSRESULT: 11
};

/**
 * @return {proto.dp.service.annotation.QueryDataSetsResponse.ResultCase}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.getResultCase = function() {
  return /** @type {proto.dp.service.annotation.QueryDataSetsResponse.ResultCase} */(jspb.Message.computeOneofCase(this, proto.dp.service.annotation.QueryDataSetsResponse.oneofGroups_[0]));
};



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryDataSetsResponse.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryDataSetsResponse} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsResponse.toObject = function(includeInstance, msg) {
  var f, obj = {
    responsetime: (f = msg.getResponsetime()) && common_pb.Timestamp.toObject(includeInstance, f),
    exceptionalresult: (f = msg.getExceptionalresult()) && common_pb.ExceptionalResult.toObject(includeInstance, f),
    datasetsresult: (f = msg.getDatasetsresult()) && proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse}
 */
proto.dp.service.annotation.QueryDataSetsResponse.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryDataSetsResponse;
  return proto.dp.service.annotation.QueryDataSetsResponse.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryDataSetsResponse} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse}
 */
proto.dp.service.annotation.QueryDataSetsResponse.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new common_pb.Timestamp;
      reader.readMessage(value,common_pb.Timestamp.deserializeBinaryFromReader);
      msg.setResponsetime(value);
      break;
    case 10:
      var value = new common_pb.ExceptionalResult;
      reader.readMessage(value,common_pb.ExceptionalResult.deserializeBinaryFromReader);
      msg.setExceptionalresult(value);
      break;
    case 11:
      var value = new proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult;
      reader.readMessage(value,proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.deserializeBinaryFromReader);
      msg.setDatasetsresult(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryDataSetsResponse.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryDataSetsResponse} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsResponse.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getResponsetime();
  if (f != null) {
    writer.writeMessage(
      1,
      f,
      common_pb.Timestamp.serializeBinaryToWriter
    );
  }
  f = message.getExceptionalresult();
  if (f != null) {
    writer.writeMessage(
      10,
      f,
      common_pb.ExceptionalResult.serializeBinaryToWriter
    );
  }
  f = message.getDatasetsresult();
  if (f != null) {
    writer.writeMessage(
      11,
      f,
      proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.serializeBinaryToWriter
    );
  }
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.repeatedFields_ = [1];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.toObject = function(includeInstance, msg) {
  var f, obj = {
    datasetsList: jspb.Message.toObjectList(msg.getDatasetsList(),
    proto.dp.service.annotation.DataSet.toObject, includeInstance)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult}
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult;
  return proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult}
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new proto.dp.service.annotation.DataSet;
      reader.readMessage(value,proto.dp.service.annotation.DataSet.deserializeBinaryFromReader);
      msg.addDatasets(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getDatasetsList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      1,
      f,
      proto.dp.service.annotation.DataSet.serializeBinaryToWriter
    );
  }
};


/**
 * repeated DataSet dataSets = 1;
 * @return {!Array<!proto.dp.service.annotation.DataSet>}
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.prototype.getDatasetsList = function() {
  return /** @type{!Array<!proto.dp.service.annotation.DataSet>} */ (
    jspb.Message.getRepeatedWrapperField(this, proto.dp.service.annotation.DataSet, 1));
};


/**
 * @param {!Array<!proto.dp.service.annotation.DataSet>} value
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult} returns this
*/
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.prototype.setDatasetsList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 1, value);
};


/**
 * @param {!proto.dp.service.annotation.DataSet=} opt_value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.DataSet}
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.prototype.addDatasets = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 1, opt_value, proto.dp.service.annotation.DataSet, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult} returns this
 */
proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult.prototype.clearDatasetsList = function() {
  return this.setDatasetsList([]);
};


/**
 * optional Timestamp responseTime = 1;
 * @return {?proto.Timestamp}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.getResponsetime = function() {
  return /** @type{?proto.Timestamp} */ (
    jspb.Message.getWrapperField(this, common_pb.Timestamp, 1));
};


/**
 * @param {?proto.Timestamp|undefined} value
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse} returns this
*/
proto.dp.service.annotation.QueryDataSetsResponse.prototype.setResponsetime = function(value) {
  return jspb.Message.setWrapperField(this, 1, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse} returns this
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.clearResponsetime = function() {
  return this.setResponsetime(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.hasResponsetime = function() {
  return jspb.Message.getField(this, 1) != null;
};


/**
 * optional ExceptionalResult exceptionalResult = 10;
 * @return {?proto.ExceptionalResult}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.getExceptionalresult = function() {
  return /** @type{?proto.ExceptionalResult} */ (
    jspb.Message.getWrapperField(this, common_pb.ExceptionalResult, 10));
};


/**
 * @param {?proto.ExceptionalResult|undefined} value
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse} returns this
*/
proto.dp.service.annotation.QueryDataSetsResponse.prototype.setExceptionalresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 10, proto.dp.service.annotation.QueryDataSetsResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse} returns this
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.clearExceptionalresult = function() {
  return this.setExceptionalresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.hasExceptionalresult = function() {
  return jspb.Message.getField(this, 10) != null;
};


/**
 * optional DataSetsResult dataSetsResult = 11;
 * @return {?proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.getDatasetsresult = function() {
  return /** @type{?proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult, 11));
};


/**
 * @param {?proto.dp.service.annotation.QueryDataSetsResponse.DataSetsResult|undefined} value
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse} returns this
*/
proto.dp.service.annotation.QueryDataSetsResponse.prototype.setDatasetsresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 11, proto.dp.service.annotation.QueryDataSetsResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryDataSetsResponse} returns this
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.clearDatasetsresult = function() {
  return this.setDatasetsresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryDataSetsResponse.prototype.hasDatasetsresult = function() {
  return jspb.Message.getField(this, 11) != null;
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.Calculations.repeatedFields_ = [2];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.Calculations.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.Calculations.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.Calculations} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.Calculations.toObject = function(includeInstance, msg) {
  var f, obj = {
    id: jspb.Message.getFieldWithDefault(msg, 1, ""),
    calculationdataframesList: jspb.Message.toObjectList(msg.getCalculationdataframesList(),
    proto.dp.service.annotation.Calculations.CalculationsDataFrame.toObject, includeInstance)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.Calculations}
 */
proto.dp.service.annotation.Calculations.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.Calculations;
  return proto.dp.service.annotation.Calculations.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.Calculations} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.Calculations}
 */
proto.dp.service.annotation.Calculations.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setId(value);
      break;
    case 2:
      var value = new proto.dp.service.annotation.Calculations.CalculationsDataFrame;
      reader.readMessage(value,proto.dp.service.annotation.Calculations.CalculationsDataFrame.deserializeBinaryFromReader);
      msg.addCalculationdataframes(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.Calculations.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.Calculations.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.Calculations} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.Calculations.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getId();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getCalculationdataframesList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      2,
      f,
      proto.dp.service.annotation.Calculations.CalculationsDataFrame.serializeBinaryToWriter
    );
  }
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.repeatedFields_ = [3];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.Calculations.CalculationsDataFrame.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.Calculations.CalculationsDataFrame} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.toObject = function(includeInstance, msg) {
  var f, obj = {
    name: jspb.Message.getFieldWithDefault(msg, 1, ""),
    datatimestamps: (f = msg.getDatatimestamps()) && common_pb.DataTimestamps.toObject(includeInstance, f),
    datacolumnsList: jspb.Message.toObjectList(msg.getDatacolumnsList(),
    common_pb.DataColumn.toObject, includeInstance)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.Calculations.CalculationsDataFrame}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.Calculations.CalculationsDataFrame;
  return proto.dp.service.annotation.Calculations.CalculationsDataFrame.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.Calculations.CalculationsDataFrame} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.Calculations.CalculationsDataFrame}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setName(value);
      break;
    case 2:
      var value = new common_pb.DataTimestamps;
      reader.readMessage(value,common_pb.DataTimestamps.deserializeBinaryFromReader);
      msg.setDatatimestamps(value);
      break;
    case 3:
      var value = new common_pb.DataColumn;
      reader.readMessage(value,common_pb.DataColumn.deserializeBinaryFromReader);
      msg.addDatacolumns(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.Calculations.CalculationsDataFrame.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.Calculations.CalculationsDataFrame} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getName();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getDatatimestamps();
  if (f != null) {
    writer.writeMessage(
      2,
      f,
      common_pb.DataTimestamps.serializeBinaryToWriter
    );
  }
  f = message.getDatacolumnsList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      3,
      f,
      common_pb.DataColumn.serializeBinaryToWriter
    );
  }
};


/**
 * optional string name = 1;
 * @return {string}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.getName = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.Calculations.CalculationsDataFrame} returns this
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.setName = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional DataTimestamps dataTimestamps = 2;
 * @return {?proto.DataTimestamps}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.getDatatimestamps = function() {
  return /** @type{?proto.DataTimestamps} */ (
    jspb.Message.getWrapperField(this, common_pb.DataTimestamps, 2));
};


/**
 * @param {?proto.DataTimestamps|undefined} value
 * @return {!proto.dp.service.annotation.Calculations.CalculationsDataFrame} returns this
*/
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.setDatatimestamps = function(value) {
  return jspb.Message.setWrapperField(this, 2, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.Calculations.CalculationsDataFrame} returns this
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.clearDatatimestamps = function() {
  return this.setDatatimestamps(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.hasDatatimestamps = function() {
  return jspb.Message.getField(this, 2) != null;
};


/**
 * repeated DataColumn dataColumns = 3;
 * @return {!Array<!proto.DataColumn>}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.getDatacolumnsList = function() {
  return /** @type{!Array<!proto.DataColumn>} */ (
    jspb.Message.getRepeatedWrapperField(this, common_pb.DataColumn, 3));
};


/**
 * @param {!Array<!proto.DataColumn>} value
 * @return {!proto.dp.service.annotation.Calculations.CalculationsDataFrame} returns this
*/
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.setDatacolumnsList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 3, value);
};


/**
 * @param {!proto.DataColumn=} opt_value
 * @param {number=} opt_index
 * @return {!proto.DataColumn}
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.addDatacolumns = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 3, opt_value, proto.DataColumn, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.Calculations.CalculationsDataFrame} returns this
 */
proto.dp.service.annotation.Calculations.CalculationsDataFrame.prototype.clearDatacolumnsList = function() {
  return this.setDatacolumnsList([]);
};


/**
 * optional string id = 1;
 * @return {string}
 */
proto.dp.service.annotation.Calculations.prototype.getId = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.Calculations} returns this
 */
proto.dp.service.annotation.Calculations.prototype.setId = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * repeated CalculationsDataFrame calculationDataFrames = 2;
 * @return {!Array<!proto.dp.service.annotation.Calculations.CalculationsDataFrame>}
 */
proto.dp.service.annotation.Calculations.prototype.getCalculationdataframesList = function() {
  return /** @type{!Array<!proto.dp.service.annotation.Calculations.CalculationsDataFrame>} */ (
    jspb.Message.getRepeatedWrapperField(this, proto.dp.service.annotation.Calculations.CalculationsDataFrame, 2));
};


/**
 * @param {!Array<!proto.dp.service.annotation.Calculations.CalculationsDataFrame>} value
 * @return {!proto.dp.service.annotation.Calculations} returns this
*/
proto.dp.service.annotation.Calculations.prototype.setCalculationdataframesList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 2, value);
};


/**
 * @param {!proto.dp.service.annotation.Calculations.CalculationsDataFrame=} opt_value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.Calculations.CalculationsDataFrame}
 */
proto.dp.service.annotation.Calculations.prototype.addCalculationdataframes = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 2, opt_value, proto.dp.service.annotation.Calculations.CalculationsDataFrame, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.Calculations} returns this
 */
proto.dp.service.annotation.Calculations.prototype.clearCalculationdataframesList = function() {
  return this.setCalculationdataframesList([]);
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.CreateAnnotationRequest.repeatedFields_ = [2,4,6,7];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.CreateAnnotationRequest.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.CreateAnnotationRequest} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateAnnotationRequest.toObject = function(includeInstance, msg) {
  var f, obj = {
    ownerid: jspb.Message.getFieldWithDefault(msg, 1, ""),
    datasetidsList: (f = jspb.Message.getRepeatedField(msg, 2)) == null ? undefined : f,
    name: jspb.Message.getFieldWithDefault(msg, 3, ""),
    annotationidsList: (f = jspb.Message.getRepeatedField(msg, 4)) == null ? undefined : f,
    comment: jspb.Message.getFieldWithDefault(msg, 5, ""),
    tagsList: (f = jspb.Message.getRepeatedField(msg, 6)) == null ? undefined : f,
    attributesList: jspb.Message.toObjectList(msg.getAttributesList(),
    common_pb.Attribute.toObject, includeInstance),
    eventmetadata: (f = msg.getEventmetadata()) && common_pb.EventMetadata.toObject(includeInstance, f),
    calculations: (f = msg.getCalculations()) && proto.dp.service.annotation.Calculations.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest}
 */
proto.dp.service.annotation.CreateAnnotationRequest.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.CreateAnnotationRequest;
  return proto.dp.service.annotation.CreateAnnotationRequest.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.CreateAnnotationRequest} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest}
 */
proto.dp.service.annotation.CreateAnnotationRequest.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setOwnerid(value);
      break;
    case 2:
      var value = /** @type {string} */ (reader.readString());
      msg.addDatasetids(value);
      break;
    case 3:
      var value = /** @type {string} */ (reader.readString());
      msg.setName(value);
      break;
    case 4:
      var value = /** @type {string} */ (reader.readString());
      msg.addAnnotationids(value);
      break;
    case 5:
      var value = /** @type {string} */ (reader.readString());
      msg.setComment(value);
      break;
    case 6:
      var value = /** @type {string} */ (reader.readString());
      msg.addTags(value);
      break;
    case 7:
      var value = new common_pb.Attribute;
      reader.readMessage(value,common_pb.Attribute.deserializeBinaryFromReader);
      msg.addAttributes(value);
      break;
    case 8:
      var value = new common_pb.EventMetadata;
      reader.readMessage(value,common_pb.EventMetadata.deserializeBinaryFromReader);
      msg.setEventmetadata(value);
      break;
    case 9:
      var value = new proto.dp.service.annotation.Calculations;
      reader.readMessage(value,proto.dp.service.annotation.Calculations.deserializeBinaryFromReader);
      msg.setCalculations(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.CreateAnnotationRequest.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.CreateAnnotationRequest} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateAnnotationRequest.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getOwnerid();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getDatasetidsList();
  if (f.length > 0) {
    writer.writeRepeatedString(
      2,
      f
    );
  }
  f = message.getName();
  if (f.length > 0) {
    writer.writeString(
      3,
      f
    );
  }
  f = message.getAnnotationidsList();
  if (f.length > 0) {
    writer.writeRepeatedString(
      4,
      f
    );
  }
  f = message.getComment();
  if (f.length > 0) {
    writer.writeString(
      5,
      f
    );
  }
  f = message.getTagsList();
  if (f.length > 0) {
    writer.writeRepeatedString(
      6,
      f
    );
  }
  f = message.getAttributesList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      7,
      f,
      common_pb.Attribute.serializeBinaryToWriter
    );
  }
  f = message.getEventmetadata();
  if (f != null) {
    writer.writeMessage(
      8,
      f,
      common_pb.EventMetadata.serializeBinaryToWriter
    );
  }
  f = message.getCalculations();
  if (f != null) {
    writer.writeMessage(
      9,
      f,
      proto.dp.service.annotation.Calculations.serializeBinaryToWriter
    );
  }
};


/**
 * optional string ownerId = 1;
 * @return {string}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getOwnerid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setOwnerid = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * repeated string dataSetIds = 2;
 * @return {!Array<string>}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getDatasetidsList = function() {
  return /** @type {!Array<string>} */ (jspb.Message.getRepeatedField(this, 2));
};


/**
 * @param {!Array<string>} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setDatasetidsList = function(value) {
  return jspb.Message.setField(this, 2, value || []);
};


/**
 * @param {string} value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.addDatasetids = function(value, opt_index) {
  return jspb.Message.addToRepeatedField(this, 2, value, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.clearDatasetidsList = function() {
  return this.setDatasetidsList([]);
};


/**
 * optional string name = 3;
 * @return {string}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getName = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 3, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setName = function(value) {
  return jspb.Message.setProto3StringField(this, 3, value);
};


/**
 * repeated string annotationIds = 4;
 * @return {!Array<string>}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getAnnotationidsList = function() {
  return /** @type {!Array<string>} */ (jspb.Message.getRepeatedField(this, 4));
};


/**
 * @param {!Array<string>} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setAnnotationidsList = function(value) {
  return jspb.Message.setField(this, 4, value || []);
};


/**
 * @param {string} value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.addAnnotationids = function(value, opt_index) {
  return jspb.Message.addToRepeatedField(this, 4, value, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.clearAnnotationidsList = function() {
  return this.setAnnotationidsList([]);
};


/**
 * optional string comment = 5;
 * @return {string}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getComment = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 5, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setComment = function(value) {
  return jspb.Message.setProto3StringField(this, 5, value);
};


/**
 * repeated string tags = 6;
 * @return {!Array<string>}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getTagsList = function() {
  return /** @type {!Array<string>} */ (jspb.Message.getRepeatedField(this, 6));
};


/**
 * @param {!Array<string>} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setTagsList = function(value) {
  return jspb.Message.setField(this, 6, value || []);
};


/**
 * @param {string} value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.addTags = function(value, opt_index) {
  return jspb.Message.addToRepeatedField(this, 6, value, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.clearTagsList = function() {
  return this.setTagsList([]);
};


/**
 * repeated Attribute attributes = 7;
 * @return {!Array<!proto.Attribute>}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getAttributesList = function() {
  return /** @type{!Array<!proto.Attribute>} */ (
    jspb.Message.getRepeatedWrapperField(this, common_pb.Attribute, 7));
};


/**
 * @param {!Array<!proto.Attribute>} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
*/
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setAttributesList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 7, value);
};


/**
 * @param {!proto.Attribute=} opt_value
 * @param {number=} opt_index
 * @return {!proto.Attribute}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.addAttributes = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 7, opt_value, proto.Attribute, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.clearAttributesList = function() {
  return this.setAttributesList([]);
};


/**
 * optional EventMetadata eventMetadata = 8;
 * @return {?proto.EventMetadata}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getEventmetadata = function() {
  return /** @type{?proto.EventMetadata} */ (
    jspb.Message.getWrapperField(this, common_pb.EventMetadata, 8));
};


/**
 * @param {?proto.EventMetadata|undefined} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
*/
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setEventmetadata = function(value) {
  return jspb.Message.setWrapperField(this, 8, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.clearEventmetadata = function() {
  return this.setEventmetadata(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.hasEventmetadata = function() {
  return jspb.Message.getField(this, 8) != null;
};


/**
 * optional Calculations calculations = 9;
 * @return {?proto.dp.service.annotation.Calculations}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.getCalculations = function() {
  return /** @type{?proto.dp.service.annotation.Calculations} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.Calculations, 9));
};


/**
 * @param {?proto.dp.service.annotation.Calculations|undefined} value
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
*/
proto.dp.service.annotation.CreateAnnotationRequest.prototype.setCalculations = function(value) {
  return jspb.Message.setWrapperField(this, 9, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateAnnotationRequest} returns this
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.clearCalculations = function() {
  return this.setCalculations(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateAnnotationRequest.prototype.hasCalculations = function() {
  return jspb.Message.getField(this, 9) != null;
};



/**
 * Oneof group definitions for this message. Each group defines the field
 * numbers belonging to that group. When of these fields' value is set, all
 * other fields in the group are cleared. During deserialization, if multiple
 * fields are encountered for a group, only the last value seen will be kept.
 * @private {!Array<!Array<number>>}
 * @const
 */
proto.dp.service.annotation.CreateAnnotationResponse.oneofGroups_ = [[10,11]];

/**
 * @enum {number}
 */
proto.dp.service.annotation.CreateAnnotationResponse.ResultCase = {
  RESULT_NOT_SET: 0,
  EXCEPTIONALRESULT: 10,
  CREATEANNOTATIONRESULT: 11
};

/**
 * @return {proto.dp.service.annotation.CreateAnnotationResponse.ResultCase}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.getResultCase = function() {
  return /** @type {proto.dp.service.annotation.CreateAnnotationResponse.ResultCase} */(jspb.Message.computeOneofCase(this, proto.dp.service.annotation.CreateAnnotationResponse.oneofGroups_[0]));
};



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.CreateAnnotationResponse.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.CreateAnnotationResponse} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateAnnotationResponse.toObject = function(includeInstance, msg) {
  var f, obj = {
    responsetime: (f = msg.getResponsetime()) && common_pb.Timestamp.toObject(includeInstance, f),
    exceptionalresult: (f = msg.getExceptionalresult()) && common_pb.ExceptionalResult.toObject(includeInstance, f),
    createannotationresult: (f = msg.getCreateannotationresult()) && proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse}
 */
proto.dp.service.annotation.CreateAnnotationResponse.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.CreateAnnotationResponse;
  return proto.dp.service.annotation.CreateAnnotationResponse.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.CreateAnnotationResponse} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse}
 */
proto.dp.service.annotation.CreateAnnotationResponse.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new common_pb.Timestamp;
      reader.readMessage(value,common_pb.Timestamp.deserializeBinaryFromReader);
      msg.setResponsetime(value);
      break;
    case 10:
      var value = new common_pb.ExceptionalResult;
      reader.readMessage(value,common_pb.ExceptionalResult.deserializeBinaryFromReader);
      msg.setExceptionalresult(value);
      break;
    case 11:
      var value = new proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult;
      reader.readMessage(value,proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.deserializeBinaryFromReader);
      msg.setCreateannotationresult(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.CreateAnnotationResponse.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.CreateAnnotationResponse} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateAnnotationResponse.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getResponsetime();
  if (f != null) {
    writer.writeMessage(
      1,
      f,
      common_pb.Timestamp.serializeBinaryToWriter
    );
  }
  f = message.getExceptionalresult();
  if (f != null) {
    writer.writeMessage(
      10,
      f,
      common_pb.ExceptionalResult.serializeBinaryToWriter
    );
  }
  f = message.getCreateannotationresult();
  if (f != null) {
    writer.writeMessage(
      11,
      f,
      proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.serializeBinaryToWriter
    );
  }
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.toObject = function(includeInstance, msg) {
  var f, obj = {
    annotationid: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult}
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult;
  return proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult}
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setAnnotationid(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getAnnotationid();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string annotationId = 1;
 * @return {string}
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.prototype.getAnnotationid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult} returns this
 */
proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult.prototype.setAnnotationid = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional Timestamp responseTime = 1;
 * @return {?proto.Timestamp}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.getResponsetime = function() {
  return /** @type{?proto.Timestamp} */ (
    jspb.Message.getWrapperField(this, common_pb.Timestamp, 1));
};


/**
 * @param {?proto.Timestamp|undefined} value
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse} returns this
*/
proto.dp.service.annotation.CreateAnnotationResponse.prototype.setResponsetime = function(value) {
  return jspb.Message.setWrapperField(this, 1, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse} returns this
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.clearResponsetime = function() {
  return this.setResponsetime(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.hasResponsetime = function() {
  return jspb.Message.getField(this, 1) != null;
};


/**
 * optional ExceptionalResult exceptionalResult = 10;
 * @return {?proto.ExceptionalResult}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.getExceptionalresult = function() {
  return /** @type{?proto.ExceptionalResult} */ (
    jspb.Message.getWrapperField(this, common_pb.ExceptionalResult, 10));
};


/**
 * @param {?proto.ExceptionalResult|undefined} value
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse} returns this
*/
proto.dp.service.annotation.CreateAnnotationResponse.prototype.setExceptionalresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 10, proto.dp.service.annotation.CreateAnnotationResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse} returns this
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.clearExceptionalresult = function() {
  return this.setExceptionalresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.hasExceptionalresult = function() {
  return jspb.Message.getField(this, 10) != null;
};


/**
 * optional CreateAnnotationResult createAnnotationResult = 11;
 * @return {?proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.getCreateannotationresult = function() {
  return /** @type{?proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult, 11));
};


/**
 * @param {?proto.dp.service.annotation.CreateAnnotationResponse.CreateAnnotationResult|undefined} value
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse} returns this
*/
proto.dp.service.annotation.CreateAnnotationResponse.prototype.setCreateannotationresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 11, proto.dp.service.annotation.CreateAnnotationResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.CreateAnnotationResponse} returns this
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.clearCreateannotationresult = function() {
  return this.setCreateannotationresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.CreateAnnotationResponse.prototype.hasCreateannotationresult = function() {
  return jspb.Message.getField(this, 11) != null;
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.QueryAnnotationsRequest.repeatedFields_ = [1];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.toObject = function(includeInstance, msg) {
  var f, obj = {
    criteriaList: jspb.Message.toObjectList(msg.getCriteriaList(),
    proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.toObject, includeInstance)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest;
  return proto.dp.service.annotation.QueryAnnotationsRequest.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.deserializeBinaryFromReader);
      msg.addCriteria(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getCriteriaList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      1,
      f,
      proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.serializeBinaryToWriter
    );
  }
};



/**
 * Oneof group definitions for this message. Each group defines the field
 * numbers belonging to that group. When of these fields' value is set, all
 * other fields in the group are cleared. During deserialization, if multiple
 * fields are encountered for a group, only the last value seen will be kept.
 * @private {!Array<!Array<number>>}
 * @const
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_ = [[10,11,12,13,14,15,16]];

/**
 * @enum {number}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.CriterionCase = {
  CRITERION_NOT_SET: 0,
  IDCRITERION: 10,
  OWNERCRITERION: 11,
  DATASETSCRITERION: 12,
  ANNOTATIONSCRITERION: 13,
  TEXTCRITERION: 14,
  TAGSCRITERION: 15,
  ATTRIBUTESCRITERION: 16
};

/**
 * @return {proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.CriterionCase}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.getCriterionCase = function() {
  return /** @type {proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.CriterionCase} */(jspb.Message.computeOneofCase(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_[0]));
};



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    idcriterion: (f = msg.getIdcriterion()) && proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.toObject(includeInstance, f),
    ownercriterion: (f = msg.getOwnercriterion()) && proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.toObject(includeInstance, f),
    datasetscriterion: (f = msg.getDatasetscriterion()) && proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.toObject(includeInstance, f),
    annotationscriterion: (f = msg.getAnnotationscriterion()) && proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.toObject(includeInstance, f),
    textcriterion: (f = msg.getTextcriterion()) && proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.toObject(includeInstance, f),
    tagscriterion: (f = msg.getTagscriterion()) && proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.toObject(includeInstance, f),
    attributescriterion: (f = msg.getAttributescriterion()) && proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion;
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 10:
      var value = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.deserializeBinaryFromReader);
      msg.setIdcriterion(value);
      break;
    case 11:
      var value = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.deserializeBinaryFromReader);
      msg.setOwnercriterion(value);
      break;
    case 12:
      var value = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.deserializeBinaryFromReader);
      msg.setDatasetscriterion(value);
      break;
    case 13:
      var value = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.deserializeBinaryFromReader);
      msg.setAnnotationscriterion(value);
      break;
    case 14:
      var value = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.deserializeBinaryFromReader);
      msg.setTextcriterion(value);
      break;
    case 15:
      var value = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.deserializeBinaryFromReader);
      msg.setTagscriterion(value);
      break;
    case 16:
      var value = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.deserializeBinaryFromReader);
      msg.setAttributescriterion(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getIdcriterion();
  if (f != null) {
    writer.writeMessage(
      10,
      f,
      proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.serializeBinaryToWriter
    );
  }
  f = message.getOwnercriterion();
  if (f != null) {
    writer.writeMessage(
      11,
      f,
      proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.serializeBinaryToWriter
    );
  }
  f = message.getDatasetscriterion();
  if (f != null) {
    writer.writeMessage(
      12,
      f,
      proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.serializeBinaryToWriter
    );
  }
  f = message.getAnnotationscriterion();
  if (f != null) {
    writer.writeMessage(
      13,
      f,
      proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.serializeBinaryToWriter
    );
  }
  f = message.getTextcriterion();
  if (f != null) {
    writer.writeMessage(
      14,
      f,
      proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.serializeBinaryToWriter
    );
  }
  f = message.getTagscriterion();
  if (f != null) {
    writer.writeMessage(
      15,
      f,
      proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.serializeBinaryToWriter
    );
  }
  f = message.getAttributescriterion();
  if (f != null) {
    writer.writeMessage(
      16,
      f,
      proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.serializeBinaryToWriter
    );
  }
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    id: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion;
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setId(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getId();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string id = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.prototype.getId = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion.prototype.setId = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    ownerid: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion;
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setOwnerid(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getOwnerid();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string ownerId = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.prototype.getOwnerid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion.prototype.setOwnerid = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    datasetid: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion;
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setDatasetid(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getDatasetid();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string dataSetId = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.prototype.getDatasetid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion.prototype.setDatasetid = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    annotationid: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion;
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setAnnotationid(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getAnnotationid();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string annotationId = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.prototype.getAnnotationid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion.prototype.setAnnotationid = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    text: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion;
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setText(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getText();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string text = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.prototype.getText = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion.prototype.setText = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    tagvalue: jspb.Message.getFieldWithDefault(msg, 1, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion;
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setTagvalue(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getTagvalue();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
};


/**
 * optional string tagValue = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.prototype.getTagvalue = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion.prototype.setTagvalue = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.toObject = function(includeInstance, msg) {
  var f, obj = {
    key: jspb.Message.getFieldWithDefault(msg, 1, ""),
    value: jspb.Message.getFieldWithDefault(msg, 2, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion;
  return proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setKey(value);
      break;
    case 2:
      var value = /** @type {string} */ (reader.readString());
      msg.setValue(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getKey();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getValue();
  if (f.length > 0) {
    writer.writeString(
      2,
      f
    );
  }
};


/**
 * optional string key = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.prototype.getKey = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.prototype.setKey = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional string value = 2;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.prototype.getValue = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 2, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion.prototype.setValue = function(value) {
  return jspb.Message.setProto3StringField(this, 2, value);
};


/**
 * optional IdCriterion idCriterion = 10;
 * @return {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.getIdcriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion, 10));
};


/**
 * @param {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.IdCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
*/
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.setIdcriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 10, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.clearIdcriterion = function() {
  return this.setIdcriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.hasIdcriterion = function() {
  return jspb.Message.getField(this, 10) != null;
};


/**
 * optional OwnerCriterion ownerCriterion = 11;
 * @return {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.getOwnercriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion, 11));
};


/**
 * @param {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.OwnerCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
*/
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.setOwnercriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 11, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.clearOwnercriterion = function() {
  return this.setOwnercriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.hasOwnercriterion = function() {
  return jspb.Message.getField(this, 11) != null;
};


/**
 * optional DataSetsCriterion dataSetsCriterion = 12;
 * @return {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.getDatasetscriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion, 12));
};


/**
 * @param {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.DataSetsCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
*/
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.setDatasetscriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 12, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.clearDatasetscriterion = function() {
  return this.setDatasetscriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.hasDatasetscriterion = function() {
  return jspb.Message.getField(this, 12) != null;
};


/**
 * optional AnnotationsCriterion annotationsCriterion = 13;
 * @return {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.getAnnotationscriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion, 13));
};


/**
 * @param {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AnnotationsCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
*/
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.setAnnotationscriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 13, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.clearAnnotationscriterion = function() {
  return this.setAnnotationscriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.hasAnnotationscriterion = function() {
  return jspb.Message.getField(this, 13) != null;
};


/**
 * optional TextCriterion textCriterion = 14;
 * @return {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.getTextcriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion, 14));
};


/**
 * @param {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TextCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
*/
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.setTextcriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 14, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.clearTextcriterion = function() {
  return this.setTextcriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.hasTextcriterion = function() {
  return jspb.Message.getField(this, 14) != null;
};


/**
 * optional TagsCriterion tagsCriterion = 15;
 * @return {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.getTagscriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion, 15));
};


/**
 * @param {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.TagsCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
*/
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.setTagscriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 15, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.clearTagscriterion = function() {
  return this.setTagscriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.hasTagscriterion = function() {
  return jspb.Message.getField(this, 15) != null;
};


/**
 * optional AttributesCriterion attributesCriterion = 16;
 * @return {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.getAttributescriterion = function() {
  return /** @type{?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion, 16));
};


/**
 * @param {?proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.AttributesCriterion|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
*/
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.setAttributescriterion = function(value) {
  return jspb.Message.setOneofWrapperField(this, 16, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.clearAttributescriterion = function() {
  return this.setAttributescriterion(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion.prototype.hasAttributescriterion = function() {
  return jspb.Message.getField(this, 16) != null;
};


/**
 * repeated QueryAnnotationsCriterion criteria = 1;
 * @return {!Array<!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion>}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.prototype.getCriteriaList = function() {
  return /** @type{!Array<!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion>} */ (
    jspb.Message.getRepeatedWrapperField(this, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion, 1));
};


/**
 * @param {!Array<!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion>} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest} returns this
*/
proto.dp.service.annotation.QueryAnnotationsRequest.prototype.setCriteriaList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 1, value);
};


/**
 * @param {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion=} opt_value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion}
 */
proto.dp.service.annotation.QueryAnnotationsRequest.prototype.addCriteria = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 1, opt_value, proto.dp.service.annotation.QueryAnnotationsRequest.QueryAnnotationsCriterion, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryAnnotationsRequest} returns this
 */
proto.dp.service.annotation.QueryAnnotationsRequest.prototype.clearCriteriaList = function() {
  return this.setCriteriaList([]);
};



/**
 * Oneof group definitions for this message. Each group defines the field
 * numbers belonging to that group. When of these fields' value is set, all
 * other fields in the group are cleared. During deserialization, if multiple
 * fields are encountered for a group, only the last value seen will be kept.
 * @private {!Array<!Array<number>>}
 * @const
 */
proto.dp.service.annotation.QueryAnnotationsResponse.oneofGroups_ = [[10,11]];

/**
 * @enum {number}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.ResultCase = {
  RESULT_NOT_SET: 0,
  EXCEPTIONALRESULT: 10,
  ANNOTATIONSRESULT: 11
};

/**
 * @return {proto.dp.service.annotation.QueryAnnotationsResponse.ResultCase}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.getResultCase = function() {
  return /** @type {proto.dp.service.annotation.QueryAnnotationsResponse.ResultCase} */(jspb.Message.computeOneofCase(this, proto.dp.service.annotation.QueryAnnotationsResponse.oneofGroups_[0]));
};



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsResponse.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsResponse.toObject = function(includeInstance, msg) {
  var f, obj = {
    responsetime: (f = msg.getResponsetime()) && common_pb.Timestamp.toObject(includeInstance, f),
    exceptionalresult: (f = msg.getExceptionalresult()) && common_pb.ExceptionalResult.toObject(includeInstance, f),
    annotationsresult: (f = msg.getAnnotationsresult()) && proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsResponse;
  return proto.dp.service.annotation.QueryAnnotationsResponse.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new common_pb.Timestamp;
      reader.readMessage(value,common_pb.Timestamp.deserializeBinaryFromReader);
      msg.setResponsetime(value);
      break;
    case 10:
      var value = new common_pb.ExceptionalResult;
      reader.readMessage(value,common_pb.ExceptionalResult.deserializeBinaryFromReader);
      msg.setExceptionalresult(value);
      break;
    case 11:
      var value = new proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.deserializeBinaryFromReader);
      msg.setAnnotationsresult(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsResponse.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsResponse.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getResponsetime();
  if (f != null) {
    writer.writeMessage(
      1,
      f,
      common_pb.Timestamp.serializeBinaryToWriter
    );
  }
  f = message.getExceptionalresult();
  if (f != null) {
    writer.writeMessage(
      10,
      f,
      common_pb.ExceptionalResult.serializeBinaryToWriter
    );
  }
  f = message.getAnnotationsresult();
  if (f != null) {
    writer.writeMessage(
      11,
      f,
      proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.serializeBinaryToWriter
    );
  }
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.repeatedFields_ = [1];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.toObject = function(includeInstance, msg) {
  var f, obj = {
    annotationsList: jspb.Message.toObjectList(msg.getAnnotationsList(),
    proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.toObject, includeInstance)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult;
  return proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation;
      reader.readMessage(value,proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.deserializeBinaryFromReader);
      msg.addAnnotations(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getAnnotationsList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      1,
      f,
      proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.serializeBinaryToWriter
    );
  }
};



/**
 * List of repeated fields within this message type.
 * @private {!Array<number>}
 * @const
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.repeatedFields_ = [3,4,6,8,9];



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.toObject = function(includeInstance, msg) {
  var f, obj = {
    id: jspb.Message.getFieldWithDefault(msg, 1, ""),
    ownerid: jspb.Message.getFieldWithDefault(msg, 2, ""),
    datasetidsList: (f = jspb.Message.getRepeatedField(msg, 3)) == null ? undefined : f,
    datasetsList: jspb.Message.toObjectList(msg.getDatasetsList(),
    proto.dp.service.annotation.DataSet.toObject, includeInstance),
    name: jspb.Message.getFieldWithDefault(msg, 5, ""),
    annotationidsList: (f = jspb.Message.getRepeatedField(msg, 6)) == null ? undefined : f,
    comment: jspb.Message.getFieldWithDefault(msg, 7, ""),
    tagsList: (f = jspb.Message.getRepeatedField(msg, 8)) == null ? undefined : f,
    attributesList: jspb.Message.toObjectList(msg.getAttributesList(),
    common_pb.Attribute.toObject, includeInstance),
    eventmetadata: (f = msg.getEventmetadata()) && common_pb.EventMetadata.toObject(includeInstance, f),
    calculations: (f = msg.getCalculations()) && proto.dp.service.annotation.Calculations.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation;
  return proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setId(value);
      break;
    case 2:
      var value = /** @type {string} */ (reader.readString());
      msg.setOwnerid(value);
      break;
    case 3:
      var value = /** @type {string} */ (reader.readString());
      msg.addDatasetids(value);
      break;
    case 4:
      var value = new proto.dp.service.annotation.DataSet;
      reader.readMessage(value,proto.dp.service.annotation.DataSet.deserializeBinaryFromReader);
      msg.addDatasets(value);
      break;
    case 5:
      var value = /** @type {string} */ (reader.readString());
      msg.setName(value);
      break;
    case 6:
      var value = /** @type {string} */ (reader.readString());
      msg.addAnnotationids(value);
      break;
    case 7:
      var value = /** @type {string} */ (reader.readString());
      msg.setComment(value);
      break;
    case 8:
      var value = /** @type {string} */ (reader.readString());
      msg.addTags(value);
      break;
    case 9:
      var value = new common_pb.Attribute;
      reader.readMessage(value,common_pb.Attribute.deserializeBinaryFromReader);
      msg.addAttributes(value);
      break;
    case 10:
      var value = new common_pb.EventMetadata;
      reader.readMessage(value,common_pb.EventMetadata.deserializeBinaryFromReader);
      msg.setEventmetadata(value);
      break;
    case 11:
      var value = new proto.dp.service.annotation.Calculations;
      reader.readMessage(value,proto.dp.service.annotation.Calculations.deserializeBinaryFromReader);
      msg.setCalculations(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getId();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getOwnerid();
  if (f.length > 0) {
    writer.writeString(
      2,
      f
    );
  }
  f = message.getDatasetidsList();
  if (f.length > 0) {
    writer.writeRepeatedString(
      3,
      f
    );
  }
  f = message.getDatasetsList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      4,
      f,
      proto.dp.service.annotation.DataSet.serializeBinaryToWriter
    );
  }
  f = message.getName();
  if (f.length > 0) {
    writer.writeString(
      5,
      f
    );
  }
  f = message.getAnnotationidsList();
  if (f.length > 0) {
    writer.writeRepeatedString(
      6,
      f
    );
  }
  f = message.getComment();
  if (f.length > 0) {
    writer.writeString(
      7,
      f
    );
  }
  f = message.getTagsList();
  if (f.length > 0) {
    writer.writeRepeatedString(
      8,
      f
    );
  }
  f = message.getAttributesList();
  if (f.length > 0) {
    writer.writeRepeatedMessage(
      9,
      f,
      common_pb.Attribute.serializeBinaryToWriter
    );
  }
  f = message.getEventmetadata();
  if (f != null) {
    writer.writeMessage(
      10,
      f,
      common_pb.EventMetadata.serializeBinaryToWriter
    );
  }
  f = message.getCalculations();
  if (f != null) {
    writer.writeMessage(
      11,
      f,
      proto.dp.service.annotation.Calculations.serializeBinaryToWriter
    );
  }
};


/**
 * optional string id = 1;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getId = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setId = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional string ownerId = 2;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getOwnerid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 2, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setOwnerid = function(value) {
  return jspb.Message.setProto3StringField(this, 2, value);
};


/**
 * repeated string dataSetIds = 3;
 * @return {!Array<string>}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getDatasetidsList = function() {
  return /** @type {!Array<string>} */ (jspb.Message.getRepeatedField(this, 3));
};


/**
 * @param {!Array<string>} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setDatasetidsList = function(value) {
  return jspb.Message.setField(this, 3, value || []);
};


/**
 * @param {string} value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.addDatasetids = function(value, opt_index) {
  return jspb.Message.addToRepeatedField(this, 3, value, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.clearDatasetidsList = function() {
  return this.setDatasetidsList([]);
};


/**
 * repeated DataSet dataSets = 4;
 * @return {!Array<!proto.dp.service.annotation.DataSet>}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getDatasetsList = function() {
  return /** @type{!Array<!proto.dp.service.annotation.DataSet>} */ (
    jspb.Message.getRepeatedWrapperField(this, proto.dp.service.annotation.DataSet, 4));
};


/**
 * @param {!Array<!proto.dp.service.annotation.DataSet>} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
*/
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setDatasetsList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 4, value);
};


/**
 * @param {!proto.dp.service.annotation.DataSet=} opt_value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.DataSet}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.addDatasets = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 4, opt_value, proto.dp.service.annotation.DataSet, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.clearDatasetsList = function() {
  return this.setDatasetsList([]);
};


/**
 * optional string name = 5;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getName = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 5, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setName = function(value) {
  return jspb.Message.setProto3StringField(this, 5, value);
};


/**
 * repeated string annotationIds = 6;
 * @return {!Array<string>}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getAnnotationidsList = function() {
  return /** @type {!Array<string>} */ (jspb.Message.getRepeatedField(this, 6));
};


/**
 * @param {!Array<string>} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setAnnotationidsList = function(value) {
  return jspb.Message.setField(this, 6, value || []);
};


/**
 * @param {string} value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.addAnnotationids = function(value, opt_index) {
  return jspb.Message.addToRepeatedField(this, 6, value, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.clearAnnotationidsList = function() {
  return this.setAnnotationidsList([]);
};


/**
 * optional string comment = 7;
 * @return {string}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getComment = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 7, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setComment = function(value) {
  return jspb.Message.setProto3StringField(this, 7, value);
};


/**
 * repeated string tags = 8;
 * @return {!Array<string>}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getTagsList = function() {
  return /** @type {!Array<string>} */ (jspb.Message.getRepeatedField(this, 8));
};


/**
 * @param {!Array<string>} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setTagsList = function(value) {
  return jspb.Message.setField(this, 8, value || []);
};


/**
 * @param {string} value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.addTags = function(value, opt_index) {
  return jspb.Message.addToRepeatedField(this, 8, value, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.clearTagsList = function() {
  return this.setTagsList([]);
};


/**
 * repeated Attribute attributes = 9;
 * @return {!Array<!proto.Attribute>}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getAttributesList = function() {
  return /** @type{!Array<!proto.Attribute>} */ (
    jspb.Message.getRepeatedWrapperField(this, common_pb.Attribute, 9));
};


/**
 * @param {!Array<!proto.Attribute>} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
*/
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setAttributesList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 9, value);
};


/**
 * @param {!proto.Attribute=} opt_value
 * @param {number=} opt_index
 * @return {!proto.Attribute}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.addAttributes = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 9, opt_value, proto.Attribute, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.clearAttributesList = function() {
  return this.setAttributesList([]);
};


/**
 * optional EventMetadata eventMetadata = 10;
 * @return {?proto.EventMetadata}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getEventmetadata = function() {
  return /** @type{?proto.EventMetadata} */ (
    jspb.Message.getWrapperField(this, common_pb.EventMetadata, 10));
};


/**
 * @param {?proto.EventMetadata|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
*/
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setEventmetadata = function(value) {
  return jspb.Message.setWrapperField(this, 10, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.clearEventmetadata = function() {
  return this.setEventmetadata(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.hasEventmetadata = function() {
  return jspb.Message.getField(this, 10) != null;
};


/**
 * optional Calculations calculations = 11;
 * @return {?proto.dp.service.annotation.Calculations}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.getCalculations = function() {
  return /** @type{?proto.dp.service.annotation.Calculations} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.Calculations, 11));
};


/**
 * @param {?proto.dp.service.annotation.Calculations|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
*/
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.setCalculations = function(value) {
  return jspb.Message.setWrapperField(this, 11, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.clearCalculations = function() {
  return this.setCalculations(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation.prototype.hasCalculations = function() {
  return jspb.Message.getField(this, 11) != null;
};


/**
 * repeated Annotation annotations = 1;
 * @return {!Array<!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation>}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.prototype.getAnnotationsList = function() {
  return /** @type{!Array<!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation>} */ (
    jspb.Message.getRepeatedWrapperField(this, proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation, 1));
};


/**
 * @param {!Array<!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation>} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult} returns this
*/
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.prototype.setAnnotationsList = function(value) {
  return jspb.Message.setRepeatedWrapperField(this, 1, value);
};


/**
 * @param {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation=} opt_value
 * @param {number=} opt_index
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.prototype.addAnnotations = function(opt_value, opt_index) {
  return jspb.Message.addToRepeatedWrapperField(this, 1, opt_value, proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.Annotation, opt_index);
};


/**
 * Clears the list making it empty but non-null.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult.prototype.clearAnnotationsList = function() {
  return this.setAnnotationsList([]);
};


/**
 * optional Timestamp responseTime = 1;
 * @return {?proto.Timestamp}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.getResponsetime = function() {
  return /** @type{?proto.Timestamp} */ (
    jspb.Message.getWrapperField(this, common_pb.Timestamp, 1));
};


/**
 * @param {?proto.Timestamp|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse} returns this
*/
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.setResponsetime = function(value) {
  return jspb.Message.setWrapperField(this, 1, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.clearResponsetime = function() {
  return this.setResponsetime(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.hasResponsetime = function() {
  return jspb.Message.getField(this, 1) != null;
};


/**
 * optional ExceptionalResult exceptionalResult = 10;
 * @return {?proto.ExceptionalResult}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.getExceptionalresult = function() {
  return /** @type{?proto.ExceptionalResult} */ (
    jspb.Message.getWrapperField(this, common_pb.ExceptionalResult, 10));
};


/**
 * @param {?proto.ExceptionalResult|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse} returns this
*/
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.setExceptionalresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 10, proto.dp.service.annotation.QueryAnnotationsResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.clearExceptionalresult = function() {
  return this.setExceptionalresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.hasExceptionalresult = function() {
  return jspb.Message.getField(this, 10) != null;
};


/**
 * optional AnnotationsResult annotationsResult = 11;
 * @return {?proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.getAnnotationsresult = function() {
  return /** @type{?proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult, 11));
};


/**
 * @param {?proto.dp.service.annotation.QueryAnnotationsResponse.AnnotationsResult|undefined} value
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse} returns this
*/
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.setAnnotationsresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 11, proto.dp.service.annotation.QueryAnnotationsResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.QueryAnnotationsResponse} returns this
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.clearAnnotationsresult = function() {
  return this.setAnnotationsresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.QueryAnnotationsResponse.prototype.hasAnnotationsresult = function() {
  return jspb.Message.getField(this, 11) != null;
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.ExportDataRequest.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.ExportDataRequest.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.ExportDataRequest} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.ExportDataRequest.toObject = function(includeInstance, msg) {
  var f, obj = {
    datasetid: jspb.Message.getFieldWithDefault(msg, 1, ""),
    calculationsspec: (f = msg.getCalculationsspec()) && common_pb.CalculationsSpec.toObject(includeInstance, f),
    outputformat: jspb.Message.getFieldWithDefault(msg, 3, 0)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.ExportDataRequest}
 */
proto.dp.service.annotation.ExportDataRequest.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.ExportDataRequest;
  return proto.dp.service.annotation.ExportDataRequest.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.ExportDataRequest} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.ExportDataRequest}
 */
proto.dp.service.annotation.ExportDataRequest.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setDatasetid(value);
      break;
    case 2:
      var value = new common_pb.CalculationsSpec;
      reader.readMessage(value,common_pb.CalculationsSpec.deserializeBinaryFromReader);
      msg.setCalculationsspec(value);
      break;
    case 3:
      var value = /** @type {!proto.dp.service.annotation.ExportDataRequest.ExportOutputFormat} */ (reader.readEnum());
      msg.setOutputformat(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.ExportDataRequest.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.ExportDataRequest.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.ExportDataRequest} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.ExportDataRequest.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getDatasetid();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getCalculationsspec();
  if (f != null) {
    writer.writeMessage(
      2,
      f,
      common_pb.CalculationsSpec.serializeBinaryToWriter
    );
  }
  f = message.getOutputformat();
  if (f !== 0.0) {
    writer.writeEnum(
      3,
      f
    );
  }
};


/**
 * @enum {number}
 */
proto.dp.service.annotation.ExportDataRequest.ExportOutputFormat = {
  EXPORT_FORMAT_UNSPECIFIED: 0,
  EXPORT_FORMAT_HDF5: 1,
  EXPORT_FORMAT_CSV: 2,
  EXPORT_FORMAT_XLSX: 3
};

/**
 * optional string dataSetId = 1;
 * @return {string}
 */
proto.dp.service.annotation.ExportDataRequest.prototype.getDatasetid = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.ExportDataRequest} returns this
 */
proto.dp.service.annotation.ExportDataRequest.prototype.setDatasetid = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional CalculationsSpec calculationsSpec = 2;
 * @return {?proto.CalculationsSpec}
 */
proto.dp.service.annotation.ExportDataRequest.prototype.getCalculationsspec = function() {
  return /** @type{?proto.CalculationsSpec} */ (
    jspb.Message.getWrapperField(this, common_pb.CalculationsSpec, 2));
};


/**
 * @param {?proto.CalculationsSpec|undefined} value
 * @return {!proto.dp.service.annotation.ExportDataRequest} returns this
*/
proto.dp.service.annotation.ExportDataRequest.prototype.setCalculationsspec = function(value) {
  return jspb.Message.setWrapperField(this, 2, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.ExportDataRequest} returns this
 */
proto.dp.service.annotation.ExportDataRequest.prototype.clearCalculationsspec = function() {
  return this.setCalculationsspec(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.ExportDataRequest.prototype.hasCalculationsspec = function() {
  return jspb.Message.getField(this, 2) != null;
};


/**
 * optional ExportOutputFormat outputFormat = 3;
 * @return {!proto.dp.service.annotation.ExportDataRequest.ExportOutputFormat}
 */
proto.dp.service.annotation.ExportDataRequest.prototype.getOutputformat = function() {
  return /** @type {!proto.dp.service.annotation.ExportDataRequest.ExportOutputFormat} */ (jspb.Message.getFieldWithDefault(this, 3, 0));
};


/**
 * @param {!proto.dp.service.annotation.ExportDataRequest.ExportOutputFormat} value
 * @return {!proto.dp.service.annotation.ExportDataRequest} returns this
 */
proto.dp.service.annotation.ExportDataRequest.prototype.setOutputformat = function(value) {
  return jspb.Message.setProto3EnumField(this, 3, value);
};



/**
 * Oneof group definitions for this message. Each group defines the field
 * numbers belonging to that group. When of these fields' value is set, all
 * other fields in the group are cleared. During deserialization, if multiple
 * fields are encountered for a group, only the last value seen will be kept.
 * @private {!Array<!Array<number>>}
 * @const
 */
proto.dp.service.annotation.ExportDataResponse.oneofGroups_ = [[10,11]];

/**
 * @enum {number}
 */
proto.dp.service.annotation.ExportDataResponse.ResultCase = {
  RESULT_NOT_SET: 0,
  EXCEPTIONALRESULT: 10,
  EXPORTDATARESULT: 11
};

/**
 * @return {proto.dp.service.annotation.ExportDataResponse.ResultCase}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.getResultCase = function() {
  return /** @type {proto.dp.service.annotation.ExportDataResponse.ResultCase} */(jspb.Message.computeOneofCase(this, proto.dp.service.annotation.ExportDataResponse.oneofGroups_[0]));
};



if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.ExportDataResponse.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.ExportDataResponse} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.ExportDataResponse.toObject = function(includeInstance, msg) {
  var f, obj = {
    responsetime: (f = msg.getResponsetime()) && common_pb.Timestamp.toObject(includeInstance, f),
    exceptionalresult: (f = msg.getExceptionalresult()) && common_pb.ExceptionalResult.toObject(includeInstance, f),
    exportdataresult: (f = msg.getExportdataresult()) && proto.dp.service.annotation.ExportDataResponse.ExportDataResult.toObject(includeInstance, f)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.ExportDataResponse}
 */
proto.dp.service.annotation.ExportDataResponse.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.ExportDataResponse;
  return proto.dp.service.annotation.ExportDataResponse.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.ExportDataResponse} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.ExportDataResponse}
 */
proto.dp.service.annotation.ExportDataResponse.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = new common_pb.Timestamp;
      reader.readMessage(value,common_pb.Timestamp.deserializeBinaryFromReader);
      msg.setResponsetime(value);
      break;
    case 10:
      var value = new common_pb.ExceptionalResult;
      reader.readMessage(value,common_pb.ExceptionalResult.deserializeBinaryFromReader);
      msg.setExceptionalresult(value);
      break;
    case 11:
      var value = new proto.dp.service.annotation.ExportDataResponse.ExportDataResult;
      reader.readMessage(value,proto.dp.service.annotation.ExportDataResponse.ExportDataResult.deserializeBinaryFromReader);
      msg.setExportdataresult(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.ExportDataResponse.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.ExportDataResponse} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.ExportDataResponse.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getResponsetime();
  if (f != null) {
    writer.writeMessage(
      1,
      f,
      common_pb.Timestamp.serializeBinaryToWriter
    );
  }
  f = message.getExceptionalresult();
  if (f != null) {
    writer.writeMessage(
      10,
      f,
      common_pb.ExceptionalResult.serializeBinaryToWriter
    );
  }
  f = message.getExportdataresult();
  if (f != null) {
    writer.writeMessage(
      11,
      f,
      proto.dp.service.annotation.ExportDataResponse.ExportDataResult.serializeBinaryToWriter
    );
  }
};





if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * Optional fields that are not set will be set to undefined.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     net/proto2/compiler/js/internal/generator.cc#kKeyword.
 * @param {boolean=} opt_includeInstance Deprecated. whether to include the
 *     JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @return {!Object}
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.prototype.toObject = function(opt_includeInstance) {
  return proto.dp.service.annotation.ExportDataResponse.ExportDataResult.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Deprecated. Whether to include
 *     the JSPB instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.dp.service.annotation.ExportDataResponse.ExportDataResult} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.toObject = function(includeInstance, msg) {
  var f, obj = {
    filepath: jspb.Message.getFieldWithDefault(msg, 1, ""),
    fileurl: jspb.Message.getFieldWithDefault(msg, 2, "")
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.dp.service.annotation.ExportDataResponse.ExportDataResult}
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.dp.service.annotation.ExportDataResponse.ExportDataResult;
  return proto.dp.service.annotation.ExportDataResponse.ExportDataResult.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.dp.service.annotation.ExportDataResponse.ExportDataResult} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.dp.service.annotation.ExportDataResponse.ExportDataResult}
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setFilepath(value);
      break;
    case 2:
      var value = /** @type {string} */ (reader.readString());
      msg.setFileurl(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.dp.service.annotation.ExportDataResponse.ExportDataResult.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.dp.service.annotation.ExportDataResponse.ExportDataResult} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getFilepath();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getFileurl();
  if (f.length > 0) {
    writer.writeString(
      2,
      f
    );
  }
};


/**
 * optional string filePath = 1;
 * @return {string}
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.prototype.getFilepath = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.ExportDataResponse.ExportDataResult} returns this
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.prototype.setFilepath = function(value) {
  return jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional string fileUrl = 2;
 * @return {string}
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.prototype.getFileurl = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 2, ""));
};


/**
 * @param {string} value
 * @return {!proto.dp.service.annotation.ExportDataResponse.ExportDataResult} returns this
 */
proto.dp.service.annotation.ExportDataResponse.ExportDataResult.prototype.setFileurl = function(value) {
  return jspb.Message.setProto3StringField(this, 2, value);
};


/**
 * optional Timestamp responseTime = 1;
 * @return {?proto.Timestamp}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.getResponsetime = function() {
  return /** @type{?proto.Timestamp} */ (
    jspb.Message.getWrapperField(this, common_pb.Timestamp, 1));
};


/**
 * @param {?proto.Timestamp|undefined} value
 * @return {!proto.dp.service.annotation.ExportDataResponse} returns this
*/
proto.dp.service.annotation.ExportDataResponse.prototype.setResponsetime = function(value) {
  return jspb.Message.setWrapperField(this, 1, value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.ExportDataResponse} returns this
 */
proto.dp.service.annotation.ExportDataResponse.prototype.clearResponsetime = function() {
  return this.setResponsetime(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.hasResponsetime = function() {
  return jspb.Message.getField(this, 1) != null;
};


/**
 * optional ExceptionalResult exceptionalResult = 10;
 * @return {?proto.ExceptionalResult}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.getExceptionalresult = function() {
  return /** @type{?proto.ExceptionalResult} */ (
    jspb.Message.getWrapperField(this, common_pb.ExceptionalResult, 10));
};


/**
 * @param {?proto.ExceptionalResult|undefined} value
 * @return {!proto.dp.service.annotation.ExportDataResponse} returns this
*/
proto.dp.service.annotation.ExportDataResponse.prototype.setExceptionalresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 10, proto.dp.service.annotation.ExportDataResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.ExportDataResponse} returns this
 */
proto.dp.service.annotation.ExportDataResponse.prototype.clearExceptionalresult = function() {
  return this.setExceptionalresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.hasExceptionalresult = function() {
  return jspb.Message.getField(this, 10) != null;
};


/**
 * optional ExportDataResult exportDataResult = 11;
 * @return {?proto.dp.service.annotation.ExportDataResponse.ExportDataResult}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.getExportdataresult = function() {
  return /** @type{?proto.dp.service.annotation.ExportDataResponse.ExportDataResult} */ (
    jspb.Message.getWrapperField(this, proto.dp.service.annotation.ExportDataResponse.ExportDataResult, 11));
};


/**
 * @param {?proto.dp.service.annotation.ExportDataResponse.ExportDataResult|undefined} value
 * @return {!proto.dp.service.annotation.ExportDataResponse} returns this
*/
proto.dp.service.annotation.ExportDataResponse.prototype.setExportdataresult = function(value) {
  return jspb.Message.setOneofWrapperField(this, 11, proto.dp.service.annotation.ExportDataResponse.oneofGroups_[0], value);
};


/**
 * Clears the message field making it undefined.
 * @return {!proto.dp.service.annotation.ExportDataResponse} returns this
 */
proto.dp.service.annotation.ExportDataResponse.prototype.clearExportdataresult = function() {
  return this.setExportdataresult(undefined);
};


/**
 * Returns whether this field is set.
 * @return {boolean}
 */
proto.dp.service.annotation.ExportDataResponse.prototype.hasExportdataresult = function() {
  return jspb.Message.getField(this, 11) != null;
};


goog.object.extend(exports, proto.dp.service.annotation);
