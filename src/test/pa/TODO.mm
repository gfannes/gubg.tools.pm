<map version="freeplane 1.2.0">
<!--To view this file, download free mind mapping software Freeplane from http://freeplane.sourceforge.net -->
<attribute_registry SHOW_ATTRIBUTES="selected"/>
<node TEXT="TODO" ID="ID_1723255651" CREATED="1283093380553" MODIFIED="1455570448614"><hook NAME="MapStyle" zoom="2.143">
    <properties show_icon_for_attributes="true" show_note_icons="true"/>

<map_styles>
<stylenode LOCALIZED_TEXT="styles.root_node">
<stylenode LOCALIZED_TEXT="styles.predefined" POSITION="right">
<stylenode LOCALIZED_TEXT="default" MAX_WIDTH="600" COLOR="#000000" STYLE="as_parent">
<font NAME="SansSerif" SIZE="10" BOLD="false" ITALIC="false"/>
</stylenode>
<stylenode LOCALIZED_TEXT="defaultstyle.details"/>
<stylenode LOCALIZED_TEXT="defaultstyle.note"/>
<stylenode LOCALIZED_TEXT="defaultstyle.floating">
<edge STYLE="hide_edge"/>
<cloud COLOR="#f0f0f0" SHAPE="ROUND_RECT"/>
</stylenode>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.user-defined" POSITION="right">
<stylenode LOCALIZED_TEXT="styles.topic" COLOR="#18898b" STYLE="fork">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.subtopic" COLOR="#cc3300" STYLE="fork">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.subsubtopic" COLOR="#669900">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.important">
<icon BUILTIN="yes"/>
</stylenode>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.AutomaticLayout" POSITION="right">
<stylenode LOCALIZED_TEXT="AutomaticLayout.level.root" COLOR="#000000">
<font SIZE="18"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,1" COLOR="#0033ff">
<font SIZE="16"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,2" COLOR="#00b439">
<font SIZE="14"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,3" COLOR="#990000">
<font SIZE="12"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,4" COLOR="#111111">
<font SIZE="10"/>
</stylenode>
</stylenode>
</stylenode>
</map_styles>
</hook>
<hook NAME="AutomaticEdgeColor" COUNTER="2"/>
<attribute NAME="allocate" VALUE="geertf"/>
<node TEXT="Product A" POSITION="right" ID="ID_1993695274" CREATED="1455570448524" MODIFIED="1455570452165">
<edge COLOR="#ff0000"/>
<node TEXT="Task A.1" ID="ID_1422220277" CREATED="1455570458346" MODIFIED="1455570495307">
<attribute NAME="estimate" VALUE="2" OBJECT="org.freeplane.features.format.FormattedNumber|2"/>
</node>
<node TEXT="Task A.2" ID="ID_306018729" CREATED="1455570463549" MODIFIED="1455570504073">
<attribute NAME="estimate" VALUE="1" OBJECT="org.freeplane.features.format.FormattedNumber|1"/>
</node>
</node>
<node TEXT="Product B" POSITION="right" ID="ID_1910502682" CREATED="1455570452588" MODIFIED="1455570455589">
<edge COLOR="#0000ff"/>
<node TEXT="Task B.1" ID="ID_1687512693" CREATED="1455570468886" MODIFIED="1455570718879">
<attribute NAME="estimate" VALUE="2" OBJECT="org.freeplane.features.format.FormattedNumber|2"/>
</node>
<node TEXT="Task B.2" ID="ID_860972045" CREATED="1455570472830" MODIFIED="1455571424526">
<attribute NAME="estimate" VALUE="3" OBJECT="org.freeplane.features.format.FormattedNumber|3"/>
<attribute NAME="deadline" VALUE="2016-02-18"/>
</node>
</node>
</node>
</map>
