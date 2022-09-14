# std::expected
Class template `expected<T, E>` is a vocabulary type which contains an expected value of type `T`, or an error `E`. The class skews towards behaving like a `T`, because its intended use is when the expected type is contained. When something unexpected occurs, more typing is required. When all is good, code mostly looks as if a `T` were being handled.

Class template `expected<T, E>` contains either:

*A value of type `T`, the expected value type; or

*A value of type `E`, an error type used when an unexpected outcome occured.

The interface can be queried as to whether the underlying value is the expected value (of type `T`) or an unexpected value (of type `E`). The original idea comes from Andrei Alexandrescu C++ and Beyond 2012: Systematic Error Handling in C++ [Alexandrescu.Expected](http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C), which he [revisited in CppCon 2018](https://www.youtube.com/watch?v=PH4WBuE1BHI&t=1825s), including mentions of this paper.

The interface and the rational are based on `std::optional`.
