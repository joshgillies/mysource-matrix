<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: metadata.inc,v 1.1 2003/12/02 04:43:25 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/**
* Paint the interface for managing metadata values
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
* @return boolean
*/
function paintMetadata(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$write_access = $owner->writeAccess('metadata');

	$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id, true);

	if (empty($schemas)) {
		$o->openSection('There are no metadata schemas applied for this asset.');
			$o->openField('&nbsp;');
			echo 'You must apply one or more schemas before entering metadata values.';
			$o->closeField();
		$o->closeField();
		return false;
	}

	if (file_exists($owner->data_path.'/metadata.php')) {
		$o->openSection('Current Metadata');
			$o->openField('&nbsp;');
				ob_start();
				require($owner->data_path.'/metadata.php');
				$metadata = ob_get_contents();
				ob_end_clean();
				echo nl2br(htmlspecialchars(trim($metadata)));
			$o->closeField();
		$o->closeSection();
	}

	$o->openSection('Enter Metadata Values');
		$o->openField('&nbsp;');
		?>
		You can use keyword replacements in place of values for metadata fields on this page.<br/>
		For a complete list of available keyword replacements <a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/metadata_field/popups/keywords.html', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520'); return false;">click here</a>
		<br/><br/>
		<span style="color:#FF0000"> * </span>denotes a required field
		<?php
		$o->closeField();
	$o->closeSection();

	foreach ($schemas as $schemaid) {
		$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) continue;
		$values = $mm->getSchemaMetadata($owner->id, $schema->id);
		$edit_fns = $schema->getEditFns();
		$edit_fns->paintInlineValueInterface($schema, $o, $values, $write_access);
	}

	return $write_access;

}//end paintMetadata()


/**
* Process the interface for managing metadata values
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @access public
* @return boolean TRUE is there is a change, false on error or no change
*/
function processMetadata(&$owner, &$o, $ei)
{
	// if we dont have write access, dont process anything
	if (!$owner->writeAccess('metadata')) return false;

	$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

	$schemas = $mm->getSchemas($owner->id, true);

	if (!empty($schemas)) {
		$new_values = Array();
		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			$edit_fns = $schema->getEditFns();
			$edit_fns->processInlineValueInterface($schema, $new_values);
		}
		if (!$mm->setMetadata($owner->id, $new_values)) return false;
	}
	return $mm->generateContentFile($owner->id);

}//end processMetadata()


?>