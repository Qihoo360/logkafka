## Features

### <a name="Filter"></a>Filter

* <a name="Regex Filter"></a>Regex Filter

  We currently support regular expression filter. You can add **regex\_filter\_pattern** through kafka-manager or php script. The default pattern value is "", i.e. no filter.

  The regular expression pattern conforms to PCRE2.

  The dropped messages will be recorded as debug level log. If needed, we will add a new property **regex\_filter\_log\_path**, and the corresponding dropped messages will be recorded into **regex\_filter\_log\_path**.





