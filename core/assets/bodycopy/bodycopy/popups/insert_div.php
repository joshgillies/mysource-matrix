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
* $Id: insert_div.php,v 1.10 2006/03/05 23:03:35 dmckee Exp $
*
*/

/**
* Insert DIV Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
header('Pragma: no-cache');
header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');

include(dirname(__FILE__).'/header.php');
?>
<script type="text/javascript" language="javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js'?>"	></script>
<script language="JavaScript" type="text/javascript">

	function popup_init() {
		var f = document.main_form;
	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]  = owner.form_element_value(f.identifier);
		data["layout_type"] = owner.form_element_value(f.layout_type);
		data["css_class"]   = owner.form_element_value(f.css_class);
		data["content_type"] = owner.form_element_value(f.content_type);
		owner.bodycopy_save_insert_div(data);
	}

</script>
<?php
	$am = $GLOBALS['SQ_SYSTEM']->am;
	$content_types = $am->getAssetTypeHierarchy('content_type');
	$default_content_type = $GLOBALS['SQ_SYSTEM']->getUserPrefs('bodycopy_container', 'SQ_DEFAULT_CONTENT_TYPE');
?>
<div class="title" style="text-align: right;"><?php echo translate('insert_div'); ?></div>

<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="divid" value="">
<table width="100%" border="0">
	<tr>
		<td>
		<fieldset>
			<legend><b><?php echo translate('identification'); ?></b></legend>
			<table style="width:100%">
				<tr>
					<td class="label"><?php echo translate('name'); ?>:</td>
					<td><input type="text" name="identifier" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		<fieldset>
			<legend><b><?php echo translate('style_information'); ?></b></legend>
			<table style="width:100%">
				<tr>
					<td class="bodycopy-popup-heading"><?php echo translate('content_type'); ?>:</td>
					<td>
						<select name="content_type" id="content_type">
							<?php
								foreach ($content_types as $type_code => $data) {
									$selected_text = '';
									if ($type_code == $default_content_type) {
										$selected_text = 'SELECTED';
									}
									?>
										<option value="<?php echo $type_code; ?>" <?php echo $selected_text; ?>><?php echo str_replace(' Content Type', '', $data['name']); ?></option>
									<?php
								}
							?>
						</select>
					</td>
				</tr>
				<tr>
					<td class="bodycopy-popup-heading"><?php echo translate('presentation'); ?>:</td>
					<td>
						<select name="layout_type">
							<option value="div" ><?php echo translate('block-level'); ?></option>
							<option value="span"><?php echo translate('inline'); ?></option>
							<option value="none"><?php echo translate('raw_html'); ?></option>
						</select>
					</td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('class'); ?>:</td>
					<td><input type="text" name="css_class" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		</td>
	</tr>
	<tr>
		<td>
		<div style="text-align: right;">
		<button type="button" name="ok" onClick="javascript: popup_save(this.form)"><?php echo translate('ok'); ?></button>
		&nbsp;
		<button type="button" name="cancel" onClick="javascript: popup_close();"><?php echo translate('cancel'); ?></button>
		</div>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__).'/footer.php'); ?>