#pragma once

#include "single-linked-list.h"
#include <iostream>

using namespace std::literals;

template <typename T, typename U>
void AssertEqualImpl(const T &t, const U &u, const std::string &t_str,
                     const std::string &u_str, const std::string &file,
                     const std::string &func, unsigned line, const std::string &hint) {
  if (t != u) {
    std::cerr << std::boolalpha;
    std::cerr << file << "("s << line << "): "s << func << ": "s;
    std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
    if (!hint.empty()) {
      std::cerr << " Hint: "s << hint;
    }
    std::cerr << std::endl;
    abort();
  }
}

#define ASSERT_EQUAL(a, b)                                                     \
AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint)                                          \
AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string &expr_str, const std::string &file,
                const std::string &func, unsigned line, const std::string &hint) {
  if (!value) {
    std::cerr << file << "("s << line << "): "s << func << ": "s;
    std::cerr << "ASSERT("s << expr_str << ") failed."s;
    if (!hint.empty()) {
      std::cerr << " Hint: "s << hint;
    }
    std::cerr << std::endl;
    abort();
  }
}

#define ASSERT(expr)                                                           \
  AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint)                                                \
  AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename TestFunc>
void RunTestImpl(const TestFunc &func, const std::string &test_name) {
  //LOG_DURATION(test_name, std::cout);
  func();
  std::cerr << "[OK] "s << test_name << '\n';
}

#define RUN_TEST(func) RunTestImpl(func, #func)

// -------- Начало модульных тестов поисковой системы ----------
struct ThrowOnCopy {
    ThrowOnCopy() = default;
    explicit ThrowOnCopy(int &copy_counter) noexcept
        : countdown_ptr(&copy_counter) {}
    ThrowOnCopy(const ThrowOnCopy &other)
        : countdown_ptr(other.countdown_ptr)  //
    {
      if (countdown_ptr) {
        if (*countdown_ptr == 0) {
          throw std::bad_alloc();
        } else {
          --(*countdown_ptr);
        }
      }
    }

    int* GetCountDownPtr() const {
      return countdown_ptr;
    }

    bool operator==(ThrowOnCopy& rhs) const {
      return GetCountDownPtr() == rhs.GetCountDownPtr();
    }

    // Присваивание элементов этого типа не требуется
    ThrowOnCopy &operator=(const ThrowOnCopy &rhs) = delete;
    // Адрес счётчика обратного отсчёта. Если не равен nullptr, то уменьшается
    // при каждом копировании. Как только обнулится, конструктор копирования
    // выбросит исключение
    int *countdown_ptr = nullptr;
  };

bool operator==(const ThrowOnCopy& lhs, const ThrowOnCopy& rhs) {
  return lhs.GetCountDownPtr() == rhs.GetCountDownPtr();
}

struct DeletionSpy {
    DeletionSpy() = default;
    explicit DeletionSpy(int &instance_counter) noexcept
        : instance_counter_ptr_(&instance_counter)  //
    {
      OnAddInstance();
    }
    DeletionSpy(const DeletionSpy &other) noexcept
        : instance_counter_ptr_(other.instance_counter_ptr_)  //
    {
      OnAddInstance();
    }

    DeletionSpy &operator=(const DeletionSpy &rhs) noexcept {
      if (this != &rhs) {
        auto rhs_copy(rhs);
        std::swap(instance_counter_ptr_, rhs_copy.instance_counter_ptr_);
      }
      return *this;
    }

    int* GetInstantCounter() const {
      return instance_counter_ptr_;
    }

    bool operator==(DeletionSpy& rhs) const {
      return GetInstantCounter() == rhs.GetInstantCounter();
    }

    ~DeletionSpy() { OnDeleteInstance(); }

   private:
    void OnAddInstance() noexcept {
      if (instance_counter_ptr_) {
        ++(*instance_counter_ptr_);
      }
    }
    void OnDeleteInstance() noexcept {
      if (instance_counter_ptr_) {
        ASSERT(*instance_counter_ptr_ != 0);
        --(*instance_counter_ptr_);
      }
    }

    int *instance_counter_ptr_ = nullptr;
  };

bool operator==(const DeletionSpy& lhs, const DeletionSpy& rhs) {
  return lhs.GetInstantCounter() == rhs.GetInstantCounter();
}

void Test1_ThrowOnCopy () {
  // Шпион, следящий за своим удалением
  ;

  // Проверка вставки в начало
  {
    SingleLinkedList<int> l;
    ASSERT(l.IsEmpty());
    ASSERT(l.GetSize() == 0u);

    l.PushFront(0);
    l.PushFront(1);
    ASSERT(l.GetSize() == 2);
    ASSERT(!l.IsEmpty());

    l.Clear();
    ASSERT(l.GetSize() == 0);
    ASSERT(l.IsEmpty());
  }

  // Проверка фактического удаления элементов
  {
    int item0_counter = 0;
    int item1_counter = 0;
    int item2_counter = 0;
    {
      SingleLinkedList<DeletionSpy> list;
      list.PushFront(DeletionSpy{item0_counter});
      list.PushFront(DeletionSpy{item1_counter});
      list.PushFront(DeletionSpy{item2_counter});

      ASSERT(item0_counter == 1);
      ASSERT(item1_counter == 1);
      ASSERT(item2_counter == 1);
      list.Clear();
      ASSERT(item0_counter == 0);
      ASSERT(item1_counter == 0);
      ASSERT(item2_counter == 0);

      list.PushFront(DeletionSpy{item0_counter});
      list.PushFront(DeletionSpy{item1_counter});
      list.PushFront(DeletionSpy{item2_counter});
      ASSERT(item0_counter == 1);
      ASSERT(item1_counter == 1);
      ASSERT(item2_counter == 1);
    }
    ASSERT(item0_counter == 0);
    ASSERT(item1_counter == 0);
    ASSERT(item2_counter == 0);
  }

  {
    bool exception_was_thrown = false;
    // Последовательно уменьшаем счётчик копирований до нуля, пока не будет
    // выброшено исключение
    for (int max_copy_counter = 5; max_copy_counter >= 0; --max_copy_counter) {
      // Создаём непустой список
      SingleLinkedList<ThrowOnCopy> list;
      list.PushFront(ThrowOnCopy{});
      try {
        int copy_counter = max_copy_counter;
        list.PushFront(ThrowOnCopy(copy_counter));
        // Если метод не выбросил исключение, список должен перейти в новое
        // состояние
        ASSERT(list.GetSize() == 2);
      } catch (const std::bad_alloc &) {
        exception_was_thrown = true;
        // После выбрасывания исключения состояние списка должно остаться
        // прежним
        ASSERT(list.GetSize() == 1);
        break;
      }
    }
    ASSERT(exception_was_thrown);
  }
}

void Test2_DeletionSpy() {
  // Итерирование по пустому списку
  {
    SingleLinkedList<int> list;
    // Константная ссылка для доступа к константным версиям begin()/end()
    const auto &const_list = list;

    // Итераторы begin и end у пустого диапазона равны друг другу
    ASSERT(list.begin() == list.end());
    ASSERT(const_list.begin() == const_list.end());
    ASSERT(list.cbegin() == list.cend());
    ASSERT(list.cbegin() == const_list.begin());
    ASSERT(list.cend() == const_list.end());
  }

  // Итерирование по непустому списку
  {
    SingleLinkedList<int> list;
    const auto &const_list = list;

    list.PushFront(1);
    ASSERT(list.GetSize() == 1u);
    ASSERT(!list.IsEmpty());

    ASSERT(const_list.begin() != const_list.end());
    ASSERT(const_list.cbegin() != const_list.cend());
    ASSERT(list.begin() != list.end());

    ASSERT(const_list.begin() == const_list.cbegin());

    ASSERT(*list.cbegin() == 1);
    *list.begin() = -1;
    ASSERT(*list.cbegin() == -1);

    const auto old_begin = list.cbegin();
    list.PushFront(2);
    ASSERT(list.GetSize() == 2);

    const auto new_begin = list.cbegin();
    ASSERT(new_begin != old_begin);
    // Проверка прединкремента
    {
      auto new_begin_copy(new_begin);
      ASSERT((++(new_begin_copy)) == old_begin);
    }
    // Проверка постинкремента
    {
      auto new_begin_copy(new_begin);
      ASSERT(((new_begin_copy)++) == new_begin);
      ASSERT(new_begin_copy == old_begin);
    }
    // Итератор, указывающий на позицию после последнего элемента, равен
    // итератору end()
    {
      auto old_begin_copy(old_begin);
      ASSERT((++old_begin_copy) == list.end());
    }
  }
  // Преобразование итераторов
  {
    SingleLinkedList<int> list;
    list.PushFront(1);
    // Конструирование ConstIterator из Iterator
    SingleLinkedList<int>::ConstIterator const_it(list.begin());
    ASSERT(const_it == list.cbegin());
    ASSERT(*const_it == *list.cbegin());

    SingleLinkedList<int>::ConstIterator const_it1;
    // Присваивание ConstIterator'у значения Iterator
    const_it1 = list.begin();
    ASSERT(const_it1 == const_it);
  }
  // Проверка оператора ->
  {
    using namespace std;
    SingleLinkedList<std::string> string_list;

    string_list.PushFront("one"s);
    ASSERT(string_list.cbegin()->length() == 3u);
    string_list.begin()->push_back('!');
    ASSERT(*string_list.begin() == "one!"s);
  }
}

void Test3_ComparsionOperators() {
    // Проверка списков на равенство и неравенство
    {
        SingleLinkedList<int> list_1;
        list_1.PushFront(1);
        list_1.PushFront(2);

        SingleLinkedList<int> list_2;
        list_2.PushFront(1);
        list_2.PushFront(2);
        list_2.PushFront(3);

        SingleLinkedList<int> list_1_copy;
        list_1_copy.PushFront(1);
        list_1_copy.PushFront(2);

        SingleLinkedList<int> empty_list;
        SingleLinkedList<int> another_empty_list;

        // Список равен самому себе
        ASSERT(list_1 == list_1);
        ASSERT(empty_list == empty_list);

        // Списки с одинаковым содержимым равны, а с разным - не равны
        ASSERT(list_1 == list_1_copy);
        ASSERT(list_1 != list_2);
        ASSERT(list_2 != list_1);
        ASSERT(empty_list == another_empty_list);
    }

    // Обмен содержимого списков
    {
        SingleLinkedList<int> first;
        first.PushFront(1);
        first.PushFront(2);

        SingleLinkedList<int> second;
        second.PushFront(10);
        second.PushFront(11);
        second.PushFront(15);

        const auto old_first_begin = first.begin();
        const auto old_second_begin = second.begin();
        const auto old_first_size = first.GetSize();
        const auto old_second_size = second.GetSize();

        first.swap(second);

        ASSERT(second.begin() == old_first_begin);
        ASSERT(first.begin() == old_second_begin);
        ASSERT(second.GetSize() == old_first_size);
        ASSERT(first.GetSize() == old_second_size);

        // Обмен при помощи функции swap
        {
            using std::swap;

            // В отсутствие пользовательской перегрузки будет вызвана функция std::swap, которая
            // выполнит обмен через создание временной копии
            swap(first, second);

            // Убеждаемся, что используется не std::swap, а пользовательская перегрузка

            // Если бы обмен был выполнен с созданием временной копии,
            // то итератор first.begin() не будет равен ранее сохранённому значению,
            // так как копия будет хранить свои узлы по иным адресам
            ASSERT(first.begin() == old_first_begin);
            ASSERT(second.begin() == old_second_begin);
            ASSERT(first.GetSize() == old_first_size);
            ASSERT(second.GetSize() == old_second_size);
        }
    }

    // Инициализация списка при помощи std::initializer_list
    {
        SingleLinkedList<int> list1{1, 2, 3, 4, 5};
        SingleLinkedList<int> list2{1, 2, 3, 4, 5};
        ASSERT(list1.GetSize() == 5);
        ASSERT(!list1.IsEmpty());
        ASSERT(std::equal(list1.begin(), list1.end(), list2.begin()));
    }

    // Лексикографическое сравнение списков
    {
        using IntList = SingleLinkedList<int>;

        ASSERT((IntList{1, 2, 3} < IntList{1, 2, 3, 1}));
        ASSERT((IntList{1, 2, 3} <= IntList{1, 2, 3}));
        ASSERT((IntList{1, 2, 4} > IntList{1, 2, 3}));
        ASSERT((IntList{1, 2, 3} >= IntList{1, 2, 3}));
    }

    // Копирование списков
    {
        const SingleLinkedList<int> empty_list{};
        // Копирование пустого списка
        {
            auto list_copy(empty_list);
            ASSERT(list_copy.IsEmpty());
        }

        SingleLinkedList<int> non_empty_list{1, 2, 3, 4};
        // Копирование непустого списка
        {
            auto list_copy(non_empty_list);

            ASSERT(non_empty_list.begin() != list_copy.begin());
            ASSERT(list_copy == non_empty_list);
        }
    }

    // Присваивание списков
    {
        const SingleLinkedList<int> source_list{1, 2, 3, 4};

        SingleLinkedList<int> receiver{5, 4, 3, 2, 1};
        receiver = source_list;
        ASSERT(receiver.begin() != source_list.begin());
        ASSERT(receiver == source_list);
    }

    // Вспомогательный класс, бросающий исключение после создания N-копии
    ;

    // Безопасное присваивание списков
    {
        SingleLinkedList<ThrowOnCopy> src_list;
        src_list.PushFront(ThrowOnCopy{});
        src_list.PushFront(ThrowOnCopy{});
        auto thrower = src_list.begin();
        src_list.PushFront(ThrowOnCopy{});

        int copy_counter = 0;  // при первом же копировании будет выброшего исключение
        thrower->countdown_ptr = &copy_counter;

        SingleLinkedList<ThrowOnCopy> dst_list;
        dst_list.PushFront(ThrowOnCopy{});
        int dst_counter = 10;
        dst_list.begin()->countdown_ptr = &dst_counter;
        dst_list.PushFront(ThrowOnCopy{});

        try {
            dst_list = src_list;
            // Ожидается исключение при присваивании
            ASSERT(false);
        } catch (const std::bad_alloc&) {
            // Проверяем, что состояние списка-приёмника не изменилось
            // при выбрасывании исключений
            ASSERT(dst_list.GetSize() == 2);
            auto it = dst_list.begin();
            ASSERT(it != dst_list.end());
            ASSERT(it->countdown_ptr == nullptr);
            ++it;
            ASSERT(it != dst_list.end());
            ASSERT(it->countdown_ptr == &dst_counter);
            ASSERT(dst_counter == 10);
        } catch (...) {
            // Других типов исключений не ожидается
            ASSERT(false);
        }
    }
}

void TestSingleList() {
  RUN_TEST(Test1_ThrowOnCopy);
  RUN_TEST(Test2_DeletionSpy);
  RUN_TEST(Test3_ComparsionOperators);
}
