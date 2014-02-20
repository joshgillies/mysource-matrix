<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: content_type_structure_tree_edit_fns.inc,v 1.9 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

/**
* Content_Type_Structure_Tree_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage squiz
*/
class Content_Type_Structure_Tree_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object	&$asset	the raw html content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			hidden_field($prefix.'_structure_value', 'bla');
			// loop through the rows to be printed, escape the 'content' for each
			$pre_serialised = $asset->attr('structure');
			if(is_null($pre_serialised)) $pre_serialised = Array();
			foreach ($pre_serialised as $index => $row) {
				$pre_serialised[$index]['content'] = str_replace("'", "\'", $row['content']);
			}
			$serialised = var_serialise($pre_serialised);
			$serialised = 'var init_data = \''.$serialised;
			$serialised = str_replace("\n", "\\n'; init_data += '", $serialised);
			$serialised = substr($serialised, 0, strrpos($serialised, ';') + 1)."\n\n";
			$format_options = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_STRUCTURE_TREE_FORMATTING');
			?>
			<style type="text/css">

				input.structure {
					width:150px;
					border: 1px solid grey;
					font-size: 8pt;
					padding-left: 3px;
				}

			</style>

			<script type="text/javascript" src="<?php echo sq_web_path('fudge'); ?>/var_serialise/var_serialise.js"></script>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/general.js"></script>
			<script type="text/javascript">

				var data_path = "<?php echo sq_web_path('data').'/asset_types/'.$asset->type(); ?>/files/";

				//Move these out to an external file
				function row()
				{
					this.indent = 0;
					this.content = '';
					this.spacer = '0';
					this.bold = '0';
					this.italic = '0';
					this.underline = '0';
				}

				function structure()
				{
					this.rows = Array();
					this.elt = '<?php echo $prefix; ?>_cont';

					this.getValue = function()
					{
						return var_serialise(this.rows);
					}

					this.init = function()
					{
						//Prints out var init_data
						<?php echo $serialised; ?>
						var un_serialised = var_unserialise(init_data);

						for (var i = 0; i < un_serialised.length; i++) {
							var a = new row();
							a.indent = un_serialised[i]['indent'];
							a.content = un_serialised[i]['content'];
							a.bold = un_serialised[i]['bold'];
							a.italic = un_serialised[i]['italic'];
							a.underline = un_serialised[i]['underline'];
							this.rows.push(a);
						}
						this.update();
					}

					this.draw = function()
					{
						var elt = document.getElementById(this.elt);
						var src = '';
						src += '<table style="width: 600px;" cellspacing="1" cellpadding="1">';

						for (var i in this.rows) {
							if(this.rows.hasOwnProperty(i)) {
							    var indent = this.rows[i].indent * 20;
							    src += '<tr>';
							    src += '<td style="border-right: 1px dotted #999999; border-bottom: 1px dotted #999999; width: 50px;">';
							    src += '<TABLE cellSpacing=0 cellPadding=0 border=0>';
							    src += '<TBODY>';
							    src += '<TR>';
							    src += '<TD rowSpan=2><IMG alt=lef src="'+data_path+'arrow_left_blue.png" border=0 onclick="<?php echo $prefix; ?>_structure.moveLeft(' + i + ');"  title="Decrease Row Indent" /></TD>';
							    src += '<TD ><IMG alt=up src="'+data_path+'arrow_up_blue.png" border=0 onclick="<?php echo $prefix; ?>_structure.moveUp(' + i + ');" title="Move Row Up Tree" /></TD>';
							    src += '<TD rowSpan=2><IMG alt=right src="'+data_path+'arrow_right_blue.png" border=0 onclick="<?php echo $prefix; ?>_structure.moveRight(' + i + ');" title="Increase Row Indent" /></TD></TR>';
							    src += '<TR>';
							    src += '<TD><IMG style="margin-top: 2px;" alt=down src="'+data_path+'arrow_down_blue.png" border=0 onclick="<?php echo $prefix; ?>_structure.moveDown(' + i + ');" title="Move Row Down Tree" ></TD></TR></TBODY></TABLE>';
							    src += '</td><td style="border-bottom: 1px dotted #999999;">';
							    var inline_style = 'style="';
							    if (this.rows[i].italic == '1') inline_style += 'font-style: italic;';
							    if (this.rows[i].bold == '1') inline_style += 'font-weight: bold;';
							    if (this.rows[i].underline == '1') inline_style += 'text-decoration: underline;';
							    inline_style += '"';
							    src += '<span style="margin-left: ' + indent + 'px; ">';
							    src += '<input class="structure" type="text" value="' + htmlspecialchars(this.rows[i].content) + '" ';
							    src += inline_style;
							    src += ' onBlur="<?php echo $prefix; ?>_structure.updateText(' + i + ', this);" />';
							    src += '</span></td><td style="margin-right: 0px; width: 55px; border-left: 1px dotted #999999;border-bottom: 1px dotted #999999;">';
							    var style_base = ' style="border: 1px solid ';

							    <?php
							    if ($format_options['bold']) {
								    ?>
								    var style_colour = 'white';
								    src += '<img src="'+data_path+'text_bold.png" onclick="<?php echo $prefix; ?>_structure.makeBold(' + i + ');" title="Bold Text" ';
								    if (this.rows[i].bold == '1') style_colour = 'red';
								    src += style_base + style_colour + ';" />';
								    <?php
							    }
							    if ($format_options['underline']) {
								    ?>
								    style_colour = 'white';
								    src += '<img src="'+data_path+'text_underlined.png" onclick="<?php echo $prefix; ?>_structure.underline(' + i + ');"  title="Underline Row Text" ';
								    if (this.rows[i].underline == '1') style_colour = 'red';
								    src += style_base + style_colour + ';" />';
								    <?php
							    }
							    if ($format_options['italic']) {
								    ?>
								    style_colour = 'white';
								    src += '<img src="'+data_path+'text_italics.png" onclick="<?php echo $prefix; ?>_structure.italicize(' + i + ');" title="Italicise Row Text" ';
								    if (this.rows[i].italic == '1') style_colour = 'red';
								    src += style_base + style_colour + ';" />';
								    <?php
							    }
							    ?>
							    src += '</td>';
							    <?php
							    if ($format_options['delete']) {
								    ?>
								    src += '<td style="border-left: 1px dotted #999999;border-bottom: 1px dotted #999999; margin-right: 0px; width: 18px;"><img src="<?php echo sq_web_path('data'); ?>/asset_types/bodycopy/images/icons/delete.png" onclick="<?php echo $prefix; ?>_structure.remove(' + i + ');" title="Delete This Row"/></td>';
								    src += '</tr>';
								    <?php
							    }
							    ?>
							}
						}
						<?php
							if ($format_options['insert']) {
								?>
								src += '<tr><td colspan="3" style=""><img src="'+data_path+'add.png" onclick="<?php echo $prefix; ?>_structure.addRow();"  title="Add New Row" />&nbsp; Add New Row: <input id="<?php echo $prefix; ?>_new_row_text" style="margin-left: 5px;" class="structure" type="text">';
								//src += '<img src="'+data_path+'add.png" onclick="<?php echo $prefix; ?>_structure.addSpacer();"/>&nbsp; Add New Spacer: <input id="<?php echo $prefix; ?>_new_spacer_seperator" style="margin-left: 5px;" class="structure" type="text">';
								src += '</td></tr>';
								<?php
							}
						?>
						src += '</table>';
						elt.innerHTML = src;
					}

					this.moveLeft = function(index)
					{
						if (this.rows[index].indent > 0) {
							this.rows[index].indent--;
						}
						this.update();
					}

					this.moveRight = function(index)
					{
						if (index != 0) {
							if (this.rows[index].indent <= this.rows[index - 1].indent) {
								this.rows[index].indent++;
								this.update();
							}
						}
					}

					this.moveUp = function(index)
					{
						if (index != 0) {
							var dummy = this.rows[index - 1];
							this.rows[index - 1] = this.rows[index];
							this.rows[index] = dummy;
						}
						this.update();
					}

					this.moveDown = function(index)
					{
						if (index != this.rows.length - 1) {
							var dummy = this.rows[index + 1];
							this.rows[index + 1] = this.rows[index];
							this.rows[index] = dummy;
						}
						this.update();
					}

					this.remove = function(index)
					{
						this.rows.splice(index, 1);
						this.update();
					}

					this.makeBold = function(index)
					{
						if (this.rows[index].bold == '1') {
							this.rows[index].bold = '0';
						} else {
							this.rows[index].bold = '1';
						}
						this.update();
					}

					this.underline = function(index)
					{
						if (this.rows[index].underline == '1') {
							this.rows[index].underline = '0';
						} else {
							this.rows[index].underline = '1';
						}
						this.update();
					}

					this.italicize = function(index)
					{
						if (this.rows[index].italic == '1') {
							this.rows[index].italic = '0';
						} else {
							this.rows[index].italic = '1';
						}
						this.update();
					}

					this.updateText = function(index, elt)
					{
						this.rows[index].content = elt.value;
						this.update();
					}

					this.addRow = function()
					{
						var new_row = new row();
						var new_doc_text = document.getElementById('<?php echo $prefix; ?>_new_row_text');
						if (new_doc_text.value == '') {
							new_row.content = 'New Row';
						} else {
							new_row.content = new_doc_text.value;
						}
						new_doc_text.value = '';
						var index = this.rows.length - 1;
						var indent = 0;
						if (index > 0) {
							indent = this.rows[index].indent;
						}
						new_row.indent = indent;
						this.rows.push(new_row);
						this.update();
					}

					this.addSpacer = function()
					{
						var new_row = new row();
						//var new_doc_text = document.getElementById('<?php echo $prefix; ?>_new_row_text');
						new_row.content = '.<br />.<br />.<br />';
						new_row.spacer = true;
						var index = this.rows.length - 1;
						var indent = 0;
						if (index > 0) {
							indent = this.rows[index].indent;
						}
						new_row.indent = indent;
						this.rows.push(new_row);
						this.update();
					}

					this.update = function()
					{
						//Get the tree into order.
						for (var i = 0; i < this.rows.length; i++) {
							//First row needs to be at the base (leftmost) of the tree
							if (i == 0 && this.rows[i].indent != 0) {
								this.rows[i].indent = 0;
							}
							if (i > 0 && this.rows[i].indent > this.rows[i - 1].indent + 1) {
								this.rows[i].indent = this.rows[i - 1].indent + 1;
							}
						}
						document.getElementById('<?php echo $prefix; ?>_structure_value').value = this.getValue();
						this.draw();
					}

				}

				var <?php echo $prefix; ?>_structure = new structure();

			</script>

			<div id="<?php echo $prefix; ?>_cont" style="width: 100%;"><br /></div>
			<script type="text/javascript">
				<?php echo $prefix; ?>_structure.init();
			</script>
			<?php
			if ($format_options['connections']) {
				check_box($prefix.'_show_trailers', '1', $asset->attr('trailers'));
				label('Show Connections between non-adjacent nodes on the frontend', $prefix.'_show_trailers');
			}
		}//end if
		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface for this Raw HTML editor
	*
	* @param array	$link	information used to create the initial link
	* @param object	&$asset	the wysiwyg content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$serialised = array_get_index($_POST, $prefix.'_structure_value', '');
			$full_data = var_unserialise($serialised);
			$asset->setAttrValue('structure', $full_data);
			$trailers = isset($_POST[$prefix.'_show_trailers']) ? TRUE : FALSE;
			$asset->setAttrValue('trailers', $trailers);
			// Generate the HTML
			$asset->setAttrValue('html', $asset->drawTree($trailers));
		}
		return TRUE;

	}//end processBackend()


}//end class

?>
